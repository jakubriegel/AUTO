#include "../project.hpp"

AUTO::AUTO(const std::string & configFile) : activeCars(0), freeCars(0), busyCars(0) { 
    // configure app
    this->activeCarsNo = 0;
    this->freeCarsNo = 0;
    this->busyCarsNo = 0;
    this->readConfig(configFile);

    // init modules of system
    std::thread web(&AUTO::server, this), sim(&AUTO::simulator, this), ord(&AUTO::orders, this);
    web.join();
    sim.join();
    ord.join();
}

void AUTO::readConfig(const std::string & configFile){
    std::ifstream configData(configFile);
    std::stringstream buffer;
    buffer << configData.rdbuf();
    auto config = nlohmann::json::parse(buffer);

    // set cars range and count battery stats
    Car::calcBatteryUsage(config["/cars/range"_json_pointer].get<unsigned int>(), config["/cars/minuteDistance"_json_pointer].get<unsigned int>());

    // set cars number and add them
    for(unsigned int i = config["/cars/number"_json_pointer].get<unsigned int>(); i > 0; i--) this->addCar();

    // set area
    // add each island
    for(const auto & island : config["/area"_json_pointer]){
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
    this->referencePoint = Position(config["/referencePoint/lat"_json_pointer].get<double>(), config["/referencePoint/lng"_json_pointer].get<double>());

}

void AUTO::server(){
    std::cout << "crow: initialing\n";
    crow::SimpleApp app;

    // main page
    CROW_ROUTE(app, "/auto")([](){
        auto page = crow::mustache::load("main.html");
        return page.render();
    });

    // get necessary cofig data
    CROW_ROUTE(app, "/auto/config")([this](){
        // create response JSON
        LOCK mutex(this->areaMutex);
        nlohmann::json response(this->area);

        return response.dump();
    });

    // get cars data [list of cars and basic stats]
    CROW_ROUTE(app, "/auto/cars")([this](){
        // create response JSON
        nlohmann::json response;

        // create temporary JSON for cars
        nlohmann::json temp;
        // get each car data and append them to the response
        LOCK mutex(this->carsMutex);
        for(Car * car : this->activeCars){
            temp["id"] = car->getId();
            temp["status"] = car->getStatus();
            temp["pos"]["lat"] = car->getPos().getLat();
            temp["pos"]["lng"] = car->getPos().getLng();
            response["cars"].push_back(temp);
        }

        // add cars stats to the response
        response["stats"]["active"] = this->activeCarsNo;
        response["stats"]["free"] = this->freeCarsNo;
        response["stats"]["busy"] = this->busyCarsNo;

        return response.dump();
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
        auto cords = nlohmann::json::parse(r.body);

        // requat route
        LOCK mutex1(this->carsMutex), mutex2(this->areaMutex);
        const unsigned int carId = this->requestRoute(Position(cords["/A/lat"_json_pointer], cords["/A/lng"_json_pointer]), Position(cords["/B/lat"_json_pointer], cords["/B/lng"_json_pointer]));

        return crow::response(std::to_string(carId));
    });

    // serve static files
    CROW_ROUTE(app, "/auto/static/<string>")([](std::string path){
        std::fstream file("static/" + path);
        std::string response = "", temp;
        while (std::getline(file, temp)) response += temp + "\n";

        return crow::response(response);
    });

    // start server
    app.port(80).multithreaded().run();
}

void AUTO::simulator(){
    std::time_t currentTime;

    // start simulation loop
    while(true){
        // get current time | in seconds since Epoch
        currentTime = std::time(0);
        {
            LOCK mutex(this->carsMutex);
            // update every car
            for(Car * car : this->activeCars) car->update(currentTime);
        }

        // wait for next update
        std::this_thread::sleep_for(std::chrono::seconds(10));
    };
}

void AUTO::orders(){
    // wait for rest of the app to initialize
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // start simulation loop
    while(true){
        // catch possible errors with getting route
        try { 
            LOCK mutex1(this->carsMutex), mutex2(this->areaMutex);

            // draw origin island
            Area * island = &(this->area[Util::randUnInt(0, this->area.size()-1)]);
            
            // draw origin position
            Position posA;
            do {
                double lat = Util::randDouble(island->getMinLat(), island->getMaxLat());
                double lng = Util::randDouble(island->getMinLng(), island->getMaxLng());
                posA = Position(lat, lng);
            } while(this->isCarAvailable(posA) != Const::CAR_AVAILABLE);

            // draw destination island
            island = &(this->area[Util::randUnInt(0, this->area.size()-1)]);
            // draw destination position
            Position posB;
            do {
                double lat = Util::randDouble(island->getMinLat(), island->getMaxLat());
                double lng = Util::randDouble(island->getMinLng(), island->getMaxLng());
                posB = Position(lat, lng);
            } while(this->isCarAvailable(posB) != Const::CAR_AVAILABLE);

            // request route
            this->requestRoute(posA, posB);
        }
        catch (...) {
            std::cout << "Orders simulation error\n";
        }
        
        // draw time of next order
        std::this_thread::sleep_for(std::chrono::seconds(Util::randUnInt(45, 100)));
    };
    
}

Car* AUTO::addCar(){
    Car * car = new Car(this);
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
            this->busyCars.erase(std::find_if(this->busyCars.begin(), this->busyCars.end(), comp));
            this->busyCarsNo = this->busyCars.size();
            break;
        case Const::FREE:
            this->freeCars.erase(std::find_if(this->freeCars.begin(), this->freeCars.end(), comp));
            this->freeCarsNo = this->freeCars.size();
            break;
    }

    // add car to correct vector
    switch (car->getStatus()) {
        case Const::JOB:
        case Const::PREJOB:
            this->setCarAsBusy(car);
            break;
        case Const::FREE:
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

Car * AUTO::getCar(const Position & A) const {
    if(this->freeCarsNo == 0) return nullptr;
    auto closest = std::make_pair(this->freeCars[0], Position::getDistance(A, this->freeCars[0]->getPos()));
    for(Car * car : this->freeCars) {
        const double distance = Position::getDistance(A, car->getPos());
        if(distance < closest.second) closest = std::make_pair(car, distance);
    }

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

const unsigned int AUTO::requestRoute(const Position & A, const Position & B){
    if(this->isCarAvailable(A) == Const::CAR_AVAILABLE && this->inArea(B)){
        // ask Google Maps for directions
        Route * route = url::getRoute(A, B);
        
        // select car
        auto * car = this->getCar(A);
        if(car != nullptr) {
            car->addJob(route);
            return car->getId();
        }
    }
    
    // no car assigned - route cancelled
    return 0; 
}