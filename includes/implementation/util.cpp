#include "../project.hpp"

void  to_json(nlohmann::json & j, const Position & p) {
    j = nlohmann::json{{"lat", p.getLat()}, {"lng", p.getLng()}};
}

const double Position::getDistance(const Position & A, const Position & B) {
    return sqrt((A.getLat() - B.getLat())*(A.getLat() - B.getLat()) + (A.getLng() - B.getLng())*(A.getLng() - B.getLng()));
}

// constructor of AreaSegment
AreaSegment::AreaSegment(const Position & _A, const Position & _B) 
    : A(_A), B(_B), a((B.getLng() - A.getLng()) / (B.getLat() - A.getLat())), b(A.getLng() - a * A.getLat()) {}

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
            data[nlohmann::json::json_pointer(path + "/duration/value")]
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
    startTime(0), step(0), timeOfNextStep(0) {

}

void Job::start(){
    startTime = std::time(0);
    timeOfNextStep = startTime+route->getStep(step).duration;
}