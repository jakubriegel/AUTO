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
        for(const auto & node : island){ currentIsland.emplace_back(node[1], node[0]); }
    }

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

    // check if any car is available at A
    CROW_ROUTE(app,"/auto/request/isAvailable").methods("POST"_method)
    ([this](const crow::request & r){
        auto A = crow::json::load(r.body);
        if (!A) return crow::response(400);
        crow::json::wvalue response;
        if(this->isCarAvailable(Position(A["lat"].i(), A["lng"].i()))) response["response"] = true;
        else response["response"] = false;
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
        case Const::STATUS_JOB:
        case Const::STATUS_PREJOB:
            this->busyCars.erase(std::find_if(this->busyCars.begin(), this->busyCars.end(), comp));
            this->busyCarsNo = this->busyCars.size();
            break;
        case Const::STATUS_FREE:
            this->freeCars.erase(std::find_if(this->freeCars.begin(), this->freeCars.end(), comp));
            this->freeCarsNo = this->freeCars.size();
            break;
    }

    // add car to correct vector
    switch (car->getStatus()) {
        case Const::STATUS_JOB:
        case Const::STATUS_PREJOB:
            this->setCarAsBusy(car);
            break;
        case Const::STATUS_FREE:
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

bool AUTO::isCarAvailable(const Position & A) const {
    for(Car * car : this->activeCars) if(car->getStatus() == Const::STATUS_FREE) return true;
    return false;
}

const unsigned int AUTO::requestRoute(const Position & A, const Position & B){
    Route * route = url::getRoute(A, B);
    // select car [temporary algorithm selects first avaible one]
    for(Car * car : this->activeCars) if(car->getStatus() == Const::STATUS_FREE) {
        car->addJob(route);
        return car->getId();
    }
    // no car assigned - route cancelled
    return 0; 
}
    

