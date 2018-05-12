#include "../project.hpp"

AUTO::AUTO(const std::string & configFile) : activeCars(0), freeCars(0), busyCars(0) { 
    // configure app
    this->activeCarsNo = 0;
    this->freeCarsNo = 0;
    this->busyCarsNo = 0;
    this->readConfig(configFile);

    // init server and simulator
    std::thread web(&AUTO::server, this), sim(&AUTO::simulator, this);
    web.join();
    sim.join();
}

void AUTO::readConfig(const std::string & configFile){
    std::ifstream configData(configFile);
    std::stringstream buffer;
    buffer << configData.rdbuf();
    auto config = nlohmann::json::parse(buffer);

    // set cars number and add them
    for(unsigned int i = config["/cars/number"_json_pointer].get<unsigned int>(); i > 0; i--) this->addCar();

    // set area
    // add each island
    for(const auto & island : config["/area"_json_pointer]){
        this->area.emplace_back();
        auto & currentIsland = this->area.back();
        // add nodes of the island
        for(const auto & node : island) currentIsland.emplace_back(node[1], node[0]);   
    }
    // calculate lines going through area nodes
    for(const auto & island : this->area) {
        areaSegments.emplace_back();
        auto & currentIsland = this->areaSegments.back();
        for(unsigned int i = 1; i < island.size(); i++) currentIsland.emplace_back(island[i-1], island[i]);
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
        const unsigned int carId = this->requestRoute(Position(cords["/A/lat"_json_pointer], cords["/A/lng"_json_pointer]), Position(cords["/B/lat"_json_pointer], cords["/B/lng"_json_pointer]));
        
        return crow::response(std::to_string(carId));
    });

    // serve static files
    CROW_ROUTE(app, "/auto/static/<string>")([](std::string path){
        std::fstream file("static/" + path);
        std::string response = "", temp;
        while (std::getline(file, temp)) response += temp;
        
        return response;
    });

    // start server
    app.port(80).multithreaded().run();
}

void AUTO::simulator(){
    for(Car * car : this->activeCars) car->printPosition();
    std::time_t currentTime;
    while(true){
        currentTime = std::time(0);
        for(Car * car : this->activeCars) car->update(currentTime);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    };
}

Car* AUTO::addCar(){
    Car * car = new Car(this);
    this->activeCars.push_back(car);
    this->activeCarsNo = this->activeCars.size();
    //this->setCarAsFree(car);

    return car;
}

void AUTO::updateCarStatus(Car * car, unsigned int old){
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
    for(const auto & island : this->areaSegments) for(const auto & segment : island){
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

STATUS AUTO::isCarAvailable(const Position & A) const {
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
    

