#include "../project.hpp"

void to_json(nlohmann::json & j, const Position & p) {
    j = nlohmann::json{{"lat", p.getLat()}, {"lng", p.getLng()}};
}

void to_json(nlohmann::json & j, const Area & a) {
    j = nlohmann::json(a.getNodes());
}

const double Position::getDistance(const Position & A, const Position & B) {
    return sqrt((A.getLat() - B.getLat())*(A.getLat() - B.getLat()) + (A.getLng() - B.getLng())*(A.getLng() - B.getLng()));
}

// constructor of AreaSegment
AreaSegment::AreaSegment(const Position & _A, const Position & _B) 
    : A(_A), B(_B), a((B.getLng() - A.getLng()) / (B.getLat() - A.getLat())), b(A.getLng() - a * A.getLat()) {}

void Area::addNode(const double & lat, const double & lng) {
    // add node
    this->nodes.emplace_back(lat, lng);

    // calculate min/max
    if(lat < this->minLat) this->minLat = lat;
    else if(lat > this->maxLat) this->maxLat = lat;
    
    if(lng < this->minLng) this->minLng = lng;
    else if(lng > this->maxLng) this->maxLng = lng;
    
    // calculate segment for this and previus node[experimental]
    //const auto & last = std::prev(this->nodes.end());
    //if(last != this->nodes.begin()) this->segments.emplace_back(*std::prev(last), *last);
}

// constructor of Route
Route::Route(nlohmann::json _data) : data(_data) {
    start = Position(data["/routes/0/legs/0/start_location/lat"_json_pointer], data["/routes/0/legs/0/start_location/lng"_json_pointer]);
    end = Position(data["/routes/0/legs/0/end_location/lat"_json_pointer], data["/routes/0/legs/0/end_location/lng"_json_pointer]);
    startName = data["/routes/0/legs/0/start_address"_json_pointer].get<std::string>();
    endName = data["/routes/0/legs/0/end_address"_json_pointer].get<std::string>();

    distance = data["/routes/0/legs/0/distance/value"_json_pointer];
    duration = data["/routes/0/legs/0/duration/value"_json_pointer];

    std::string path;
    for(unsigned int i = 0; i < data["/routes/0/legs/0/steps"_json_pointer].size(); i++){
        path = "/routes/0/legs/0/steps/" + std::to_string(i);
        steps.emplace_back(
            Position(data[nlohmann::json::json_pointer(path + "/start_location/lat")], data[nlohmann::json::json_pointer(path + "/start_location/lng")]),
            Position(data[nlohmann::json::json_pointer(path + "/end_location/lat")], data[nlohmann::json::json_pointer(path + "/end_location/lng")]),
            data[nlohmann::json::json_pointer(path + "/distance/value")], data[nlohmann::json::json_pointer(path + "/duration/value")]
            );
    }
    stepsN = steps.size();
};

void Route::print(bool extended) const {
    std::cout << "Route:\n";
    std::cout << "\torigin: " << startName << " " << start.getLat() << " " << start.getLng() << "\n";
    std::cout << "\tdest: " << endName << " " << end.getLat() << " " << end.getLng() << "\n";
    std::cout << "\tduration: " << duration << "\tdistance: " << distance << "\n";
    if(extended){
        std::cout << "steps:\n";
        for(const Step & i : steps) std::cout << "\t" << i.duration << " " << i.start.getLat() << " " << i.start.getLng() << "\n";
    }
};

// constructor of Job
Job::Job(Route * _route)
    : route(_route), origin(route->getStart()), destination(route->getEnd()),
    startTime(0), step(0), timeOfNextStep(0), pre(nullptr) {

}

void Job::start(){
    // if there is a prejob to be done start it
    if(this->pre != nullptr) this->pre->start();
    // if not start this job
    else {
        this->startTime = std::time(0);
        this->timeOfNextStep = startTime + route->getStep(step).duration;
    }
}

// Util namespace
double Util::randDouble(double min, double max) {
    std::mt19937 generator;
    generator.seed(std::random_device()());
    std::uniform_real_distribution<double> d(min, max);
    
    return d(generator);
}

unsigned int Util::randUnInt(unsigned int min, unsigned int max) {
    std::mt19937 generator;
    generator.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> d(min, max);

    return d(generator);
}