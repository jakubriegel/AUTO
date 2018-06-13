#include "../project.hpp"

AUTO::AUTO(const std::string & configFile) : activeCars(0), freeCars(0), busyCars(0), takenRides(0), averagePreTime(0) { 
    // configure app
    this->init(configFile);

    // init modules of the system
    std::thread web(&AUTO::server, this), sim(&AUTO::simulator, this), ord(&AUTO::orders, this);
    Util::log("app ready");

    web.join();
    sim.join();
    ord.join();
}

void AUTO::init(const std::string & configFile){
    Util::log("configuring app");

    std::ifstream configData(configFile);
    std::stringstream buffer;
    buffer << configData.rdbuf();
    auto config = nlohmann::json::parse(buffer);

    // init cars stats
    this->activeCarsNo = 0;
    this->freeCarsNo = 0;
    this->busyCarsNo = 0;

    // set up the port
    this->port = new Port(
        this,
        config["/base/port/name"_json_pointer].get<std::string>(), 
        Position(config["/base/port/localization/lng"_json_pointer].get<double>(), config["/base/port/localization/lat"_json_pointer].get<double>()),
        config["/base/port/radius"_json_pointer].get<unsigned int>(),
        config["/car/number"_json_pointer].get<unsigned int>()
    );

    // get cars range and count battery stats
    Car::calcBatteryUsage(config["/car/range"_json_pointer].get<unsigned int>(), config["/car/minuteDistance"_json_pointer].get<unsigned int>());

    // get cars number and add them
    for(unsigned int i = config["/car/number"_json_pointer].get<unsigned int>(); i > 0; i--) this->addCar();
    // dock cars in the port
    for(auto car : this->activeCars) this->sendToPort(car);

    // get bases data
    for(const auto & base : config["/base/bases"_json_pointer]) {
        this->bases.emplace_back(
            this, base["/name"_json_pointer].get<std::string>(), 
            Position(base["/localization/lng"_json_pointer].get<double>(), base["/localization/lat"_json_pointer].get<double>()),
            base["/radius"_json_pointer].get<unsigned int>(),
            base["/slots"_json_pointer].get<unsigned int>()
        );
    }

    // get charging data
    Base::setCharging(config["/base/chargeInterval"_json_pointer].get<unsigned int>(), config["/base/chargeAmmount"_json_pointer].get<unsigned int>());
    
    // get area
    // add each island
    for(const auto & island : config["/area/islands"_json_pointer]){
        this->area.emplace_back();
        auto & currentIsland = this->area.back();
        // add nodes of the island
        for(const auto & node : island) currentIsland.addNode(node[1], node[0]); 
    }
    // calculate nodes segments
    for(auto & island : this->area) {
        auto & nodes = island.getNodes();
        for(unsigned int i = 1; i < nodes.size(); i++) island.addSegment(nodes[i-1], nodes[i]);
    }

    // add reference point
    this->referencePoint = Position(config["/area/referencePoint/lat"_json_pointer].get<double>(), config["/area/referencePoint/lng"_json_pointer].get<double>());

    // get simulator data
    Const::setSimulatorInterval(config["/simulator/interval"_json_pointer].get<unsigned int>());

    // get orders generator data
    Const::setGeneratorDelay(config["/ordersGenerator/delay"_json_pointer].get<unsigned int>());
    Const::setGeneratorIntervalMin(config["/ordersGenerator/intervalMin"_json_pointer].get<unsigned int>());
    Const::setGeneratorIntervalMax(config["/ordersGenerator/intervalMax"_json_pointer].get<unsigned int>());
    Const::setGeneratorMainProb(config["/ordersGenerator/mainProb"_json_pointer].get<double>());

    // get Google Maps API key
    Const::setMapsApiKey(config["/maps/apiKey"_json_pointer].get<std::string>());

    Util::log("app configured");
}

void AUTO::server(){
    crow::SimpleApp app;

    // main page
    CROW_ROUTE(app, "/auto")([](){
        auto page = crow::mustache::load("main.html");
        crow::mustache::context ctx;
        ctx["googleMapsApiKey"] = Const::getMapsApiKey();
        return page.render(ctx);
    });

    // get necessary cofig data
    CROW_ROUTE(app, "/auto/config")([this](){
        // create response JSON
        LOCK mutex(this->areaMutex);
        nlohmann::json response(this->area);

        // create response with appropiate header
        auto res = crow::response(response.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // get system` data [list of cars, bases and basic stats]
    CROW_ROUTE(app, "/auto/update")([this](){
        // create response JSON
        nlohmann::json response;

        { 
            LOCK mutex1(this->carsMutex);
            LOCK mutex2(this->basesMutex);
            // get port data and append it to the response
            response["port"] = nlohmann::json(*(this->port));
            // get each base data and append them to the response
            for(const Base & base : this->bases) response["bases"].push_back(nlohmann::json(base));
            // get each car data and append them to the response
            for(Car * const car : this->activeCars) response["cars"].push_back(nlohmann::json(*car));
            
            // add cars stats to the response
            response["stats"]["active"] = this->activeCarsNo;
            response["stats"]["free"] = this->freeCarsNo;
            response["stats"]["busy"] = this->busyCarsNo;
            response["stats"]["rides"] = this->takenRides;
            response["stats"]["preTime"] = this->averagePreTime;
        }

        // create response with appropiate header
        auto res = crow::response(response.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // check if A is in allowed area and if any car is available
    CROW_ROUTE(app,"/auto/request/isAvailable").methods("POST"_method)
    ([this](const crow::request & r){
        const auto A = crow::json::load(r.body);
        if(!A) return crow::response(400);
        crow::json::wvalue response;
        LOCK mutex1(this->carsMutex), mutex2(this->areaMutex);
        response["response"] = this->isCarAvailable(Position(A["lat"].d(), A["lng"].d()));
        return crow::response(response);
    });

    // request route
    CROW_ROUTE(app,"/auto/request/route").methods("POST"_method)
    ([this](const crow::request & r){
        // in case of errors
        auto req = crow::json::load(r.body);
        if (!req) return crow::response(400);

        // resolving A and B
        auto coords = nlohmann::json::parse(r.body);

        // request route
        LOCK mutex1(this->carsMutex), mutex2(this->areaMutex);
        auto car = this->requestRoute(Position(coords["/A/lat"_json_pointer], coords["/A/lng"_json_pointer]), Position(coords["/B/lat"_json_pointer], coords["/B/lng"_json_pointer]));

        // create response with appropiate header
        auto response = nlohmann::json();
        if(car != nullptr){
            response["status"] = Const::OK;
            response["job"] = car->getJobJson();
        }
        else response["status"] = Const::ERROR;
        auto res = crow::response(response.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // serve static files
    CROW_ROUTE(app, "/auto/static/<string>")([](std::string path){
        std::fstream file("static/" + path);
        std::string response = "", temp;
        while (std::getline(file, temp)) response += temp + "\n";

        return crow::response(response);
    });

    // start server
    Util::log("crow: starting");
    app.port(80).multithreaded().run();
}

void AUTO::simulator(){
    std::time_t currentTime;

    // start simulation loop
    while(true){
        // get current time | in seconds since Epoch
        currentTime = std::time(0);
        {
            LOCK mutex1(this->carsMutex);
            LOCK mutex2(this->basesMutex);
            LOCK mutex3(this->areaMutex);

            // update every car
            Util::log("simulator: updating cars");
            for(Car * car : this->activeCars) car->update(currentTime);
            
            // update every base
            Util::log("simulator: updating bases");
            for(Base & base : this->bases) base.update(currentTime);

            // update port
            Util::log("simulator: updating port");
            this->port->update(currentTime);
        }

        // wait for next update
        std::this_thread::sleep_for(std::chrono::seconds(Const::getSimulatorInterval()));
    };
}

void AUTO::orders(){
    // wait for rest of the app to initialize
    std::this_thread::sleep_for(std::chrono::seconds(Const::getGeneratorDelay()));

    auto drawIsland = [this]() {
        static const auto prob = 1 - Const::getGeneratorMainProb();
        const unsigned int n = (100 * (this->area.size()-1)) / (prob * 100);
        unsigned int i = Util::randUnInt(0, n);
        if(i >= this->area.size()) i = 0;   
        return &(this->area[i]);
    };

    // start simulation loop
    while(true){
        {
            LOCK mutex1(this->carsMutex), mutex2(this->areaMutex);
            // check if any car is available
            if(this->freeCarsNo > 0) {
                // catch possible errors with getting route
                try {
                    // draw origin island
                    const Area * island = drawIsland();
                    
                    // draw origin position
                    Position posA;
                    do {
                        double lat = Util::randDouble(island->getMinLat(), island->getMaxLat());
                        double lng = Util::randDouble(island->getMinLng(), island->getMaxLng());
                        posA = Position(lat, lng);
                    } while(this->isCarAvailable(posA) != Const::CAR_AVAILABLE);

                    // draw destination island | probability for it to be Poznan is 80%
                    island = drawIsland();

                    // draw destination position
                    Position posB;
                    do {
                        double lat = Util::randDouble(island->getMinLat(), island->getMaxLat());
                        double lng = Util::randDouble(island->getMinLng(), island->getMaxLng());
                        posB = Position(lat, lng);
                    } while(this->isCarAvailable(posB) != Const::CAR_AVAILABLE);

                    // request route
                    auto car = this->requestRoute(posA, posB);

                    Util::log("orders sim: car: " + std::to_string(car->getId()) + "\tfrom:\t" + std::to_string(posA.getLat()) + " " + std::to_string(posA.getLng()) 
                        + "\tto: " + std::to_string(posB.getLat()) + " " + std::to_string(posB.getLng()));
                }
                catch (...) {
                    Util::log("orders simulation error", true);
                }
            }
        }
        // draw time of next order
        std::this_thread::sleep_for(std::chrono::seconds(Util::randUnInt(Const::getGeneratorIntervalMin(), Const::getGeneratorIntervalMax())));
    };
    
}

Car * AUTO::addCar(){
    auto * car = new Car(this, this->port->getPosition());
    this->activeCars.push_back(car);
    this->activeCarsNo = this->activeCars.size();

    return car;
}

void AUTO::updateCarStatus(Car * car, const STATUS & old){
    // create compare for std::find
    const unsigned int & id = car->getId();
    auto comp = [&id](const Car * a){ return a->getId() == id; };

    // delete car from current vector
    switch (old) {
        case Const::JOB:
        case Const::PREJOB:
        case Const::TO_BASE:
        case Const::CHARGING:
            this->busyCars.erase(std::find_if(this->busyCars.begin(), this->busyCars.end(), comp));
            this->busyCarsNo = this->busyCars.size();
            break;
        case Const::FREE:
        case Const::IN_BASE:
        case Const::IN_PORT:
            this->freeCars.erase(std::find_if(this->freeCars.begin(), this->freeCars.end(), comp));
            this->freeCarsNo = this->freeCars.size();
            break;
    }

    // add car to correct vector
    switch (car->getStatus()) {
        case Const::JOB:
        case Const::PREJOB:
        case Const::TO_BASE:
        case Const::CHARGING:
            this->setCarAsBusy(car);
            break;
        case Const::FREE:
        case Const::IN_BASE:
        case Const::IN_PORT:
            this->setCarAsFree(car);
            break;
    }
}

void AUTO::setCarAsFree(Car * car){
    this->freeCars.push_back(car);
    this->freeCarsNo = this->freeCars.size();
}

void AUTO::setCarAsBusy(Car * car){
    this->busyCars.push_back(car);
    this->busyCarsNo = this->busyCars.size();
}

const bool AUTO::inArea(const Position & A) const {
    const double refA = (this->referencePoint.getLng() - A.getLng()) / (this->referencePoint.getLat() - A.getLat());
    const double refB = A.getLng() - refA * A.getLat();
    // set boundaries of reference segment
    const double refX1 = (this->referencePoint.getLat() < A.getLat()) ? this->referencePoint.getLat() : A.getLat();
    const double refX2 = (refX1 == this->referencePoint.getLat()) ? A.getLat() : this->referencePoint.getLat();

    // count intersections with area borders
    unsigned int n = 0;
    for(const auto & island : this->area) for(const auto & segment : island.getSegments()) { 
        const double x1 = (segment.A.getLat() < segment.B.getLat()) ? segment.A.getLat() : segment.B.getLat();
        const double x2 = (x1 == segment.A.getLat()) ? segment.B.getLat() : segment.A.getLat();

        // check if common range exists in given ranges
        if(refX2 >= x1 || x2 >= refX1) {
            // check if segments are collinear
            if(refA == segment.a && refB == segment.b){ 
                n++;
                continue;
            }
            // chceck if intersection exists
            if(refA == segment.a) continue; // lines that aren't collinear and have the same slope don't have instersecions
            // calculate intersection x coord and chceck if it is in common range 
            const double x = (refB - segment.b) / (segment.a - refA); 
            if(x >= refX1 && x <= refX2 && x >= x1 && x <= x2) n++;
        }
    }

    if(n % 2 == 0) return false;
    return true;
}

Car * const AUTO::getCar(const Position & A, const STATUS & exluded, const unsigned int & radius) const {
    // check if any car is available
    if(this->freeCarsNo == 0) return nullptr;
    // look for the closest car
    auto closest = std::make_pair(this->freeCars[0], DBL_MAX);
    for(Car * const car : this->freeCars) {
        const double distance = Position::getDistance(A, car->getPos());
        if(distance < closest.second) {
            if(exluded != 0) if(car->getStatus() == exluded || distance > radius) continue;
            closest = std::make_pair(car, distance);
        }
    }

    // check if any car was found
    if(closest.second == DBL_MAX) return nullptr;
    
    // return pointer to the closest car
    return closest.first;
}

const STATUS AUTO::isCarAvailable(const Position & A) const {
    // check is position is in allowed area
    if(!this->inArea(A)) return Const::OUTSIDE_ALLOWED_AREA;
    // check for available car
    if(this->getCar(A) != nullptr) return Const::CAR_AVAILABLE;
    // if no car is found
    return Const::NO_CAR_AVAILABLE;
}

Car * const AUTO::requestRoute(const Position & A, const Position & B){
    // check if car is available at A and if B is in alowed area
    if(this->isCarAvailable(A) == Const::CAR_AVAILABLE && this->inArea(B)){
        // ask Google Maps for directions
        Route * route = url::getRoute(A, B);
        // select car
        auto * car = this->getCar(A);
        if(car != nullptr) {
            car->addJob(route);
            return car;
        }
    }
    
    // no car assigned - route cancelled
    return nullptr; 
}

Car * AUTO::requestToBase(Base * base, const unsigned int & radius) {
    // check if any car is free
    if(this->isCarAvailable(base->getPosition()) == Const::CAR_AVAILABLE){
        // select car
        auto exluded = (typeid(*base).name() == typeid(Port).name()) ? Const::IN_PORT : Const::IN_BASE;
        auto * car = this->getCar(base->getPosition(), exluded, radius);
        // if no car is in the radius
        if(car == nullptr) return nullptr;  
        // send car to the base
        this->sendToBase(car, base);
        // return ordered car id
        return car;
    }
    // if no car is free
    else return nullptr;
    // the app does not consider OUTSIDE_ALLOWED_AREA, because bases are always in allowed area
}

void AUTO::sendToBase(Car * car, Base * base) {
    // ask Google Maps for directions
    Route * route = url::getRoute(car->getPos(), base->getPosition());
    // set base name as destination name
    route->setEndName("b. " + base->getName());
    // assign route
    car->addJob(route, Const::TO_BASE, Const::TO_BASE);
}  

void AUTO::sendToPort(Car * car) {
    this->port->reserveSlot(car);
    if(car->getPos() != this->port->getPosition()) sendToBase(car, this->port);
}

void AUTO::sendToClosestBase(Car * car){
    // slect closest base with free slots
    auto closest = std::make_pair(&(this->bases[0]), DBL_MAX);
    for(Base & base : this->bases){
        const double distance = Position::getDistance(car->getPos(), base.getPosition());
        if(distance < closest.second) if(base.getFreeSlots() > 0) closest = std::make_pair(&base, distance);
    }
    // if no base was available send the car to the port
    if(closest.second == DBL_MAX){
        this->sendToPort(car);
        return;
    }
    // if base was found send car to it
    this->sendToBase(car, closest.first);
}