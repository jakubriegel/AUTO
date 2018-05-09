#include "../project.hpp"

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

// static members of Car
unsigned int Car::inUse = 0;
unsigned short Car::range = 0;
unsigned char Car::perMeter = 0;

void Car::countBatteryUsage(unsigned short _range){
    Car::range = _range * 10000;
    Car::perMeter = 1000000 / Car::range;
}

void Car::init(){

}

// constructor of Car
Car::Car(AUTO * _app)
    : app(_app), id(++inUse), position(Const::BASE),
    job(nullptr), preJob(nullptr), battery(100), mileage(0) {
        this->setStatus(Const::STATUS_FREE);
};

void Car::addJob(Route * route){
    this->job = new Job(route);
    if(this->position == this->job->origin){
        this->setStatus(Const::STATUS_JOB);
        this->job->start();
    }
    else{
        this->setStatus(Const::STATUS_PREJOB);
        this->preJob = new Job(url::getRoute(this->position, this->job->origin));
        this->preJob->start();
    }
}

void Car::setStatus(unsigned int newStatus){
    unsigned int old = this->status;
    this->status = newStatus;
    this->app->updateCarStatus(this, old);
}

void Car::printPosition(){
    std::cout << "car " << this->id << " " << this->status << " position:\t" << this->position.getLat() << "\t" << this->position.getLng() << "\n";
}

void Car::update(const std::time_t & currentTime){
    switch (this->status) {
        case Const::STATUS_FREE:
        break;
        case Const::STATUS_JOB:
            if(currentTime >= this->job->timeOfNextStep){
                printPosition();
                if(++this->job->step == this->job->route->getStepsNumber()){
                    this->position = this->job->route->getStep(job->step-1).end;
                    std::cout << "car " << this->id << " arrived at the destination\n";
                    this->setStatus(Const::STATUS_FREE);
                    delete this->job;
                    this->job = nullptr;
                }
                else{
                    this->position = this->job->route->getStep(this->job->step).start;
                    this->job->timeOfNextStep += this->job->route->getStep(this->job->step).duration;
                }
            }
        break;
        case Const::STATUS_PREJOB:
            if(currentTime >= this->preJob->timeOfNextStep){
                this->printPosition();
                if(++this->preJob->step == this->preJob->route->getStepsNumber()){
                    this->position = this->preJob->route->getStep(preJob->step-1).end;
                    std::cout << "car " << this->id << " ready to start job\n";
                    this->setStatus(Const::STATUS_JOB);
                    this->job->start();
                    delete this->preJob;
                    this->preJob = nullptr;
                }
                else{
                    this->position = this->preJob->route->getStep(this->preJob->step).start;
                    this->preJob->timeOfNextStep += this->preJob->route->getStep(this->preJob->step).duration;
                }
            }
        break;
    }
}