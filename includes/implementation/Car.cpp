#include "../project.hpp"

// static members of Car
unsigned int Car::inUse = 0;
unsigned int Car::range = 0;
unsigned int Car::perMeter = 0;
unsigned int Car::lowBattery = 0;
unsigned int Car::criticalBattery = 0;
unsigned int Car::minuteDistace = 0;

void Car::calcBatteryUsage(unsigned int _range, unsigned int _minute){
    // convert range to meters and assign it
    Car::range = _range * 1000;
    // assign per meter usage by dividing 100% (times 10000) by range
    Car::perMeter = 1000000 / Car::range;
    // calculate low and critical battery %
    Car::lowBattery = Car::perMeter * 50000;
    Car::criticalBattery = Car::perMeter * 15000;
    // assign stand-by usage
    Car::minuteDistace = _minute;

    // print results
    Util::log("Car battery initialized:\trange: " + std::to_string(Car::range) 
        + "\tlowBattery: " + std::to_string(Car::lowBattery)
        + "\tcriticalBattery: " + std::to_string(Car::criticalBattery));
}

void Car::init(){

}

// constructor of Car
Car::Car(AUTO * _app, const Position & _position)
    : app(_app), id(++inUse), position(_position),
    job(nullptr), preJob(nullptr), battery(1000000), 
    mileage(0), timeOfNextBatteryUpdate(std::time(0) + Const::MINUTE) {
        this->setStatus(Const::IN_BASE);
};


void Car::setStatus(const STATUS & newStatus) {
    const STATUS old = this->status;
    this->status = newStatus;
    // let the app know about change
    this->app->updateCarStatus(this, old);
}

void Car::addWarning(const STATUS & warning) {
    // check if warnig is not already on the list and then add it
    if(std::find(this->warnigs.begin(), this->warnigs.end(), warning) == this->warnigs.end()){
        this->warnigs.push_back(warning);
        Util::log("car " + std::to_string(this->id) + ":\tnew warning: " + std::to_string(warning));
    }
}

const STATUS Car::updateBattery() {
    // get current time and call actual function
    return this->updateBattery(std::time(0));
}

const STATUS Car::updateBattery(const std::time_t & currentTime) {
    // check if it is time for update
    while(currentTime >= this->timeOfNextBatteryUpdate) {
        // subtrack used battery
        this->battery -= Car::perMeter * Car::minuteDistace;
        // schedule next update
        this->timeOfNextBatteryUpdate = currentTime + Const::MINUTE;
    }

    // check if warnings are neccesary
    if(this->battery < Car::lowBattery) {
        STATUS warning;
        if(this->battery < Car::criticalBattery) warning = Const::CRITICAL_BATTERY;
        else warning = Const::LOW_BATTERY;
        this->addWarning(warning);
        return warning;
    }

    // if no warnings
    return Const::OK;
}

const STATUS Car::updateBattery(const unsigned int & driven, const std::time_t & currentTime) {
    // calculate new battery %
    this->battery -= Car::perMeter * driven;
    // perform standard battery update
    return this->updateBattery(currentTime);
}

void Car::addJob(Route * route){
    this->addJob(route, Const::JOB, Const::PREJOB);
}

void Car::addJob(Route * route, const STATUS & status, const STATUS & statusPre) {
    // assign job
    this->job = new Job(route);
    // add it to stats
    if(status == Const::JOB) app->addRideToStats();
    // if the car is in the start point or base is the start point set its status to 'status'
    if(this->position == this->job->origin || status == statusPre) this->setStatus(status);
    // if not create prejob and set 'statusPre'
    else{
        this->setStatus(statusPre);
        this->job->pre = new Job(url::getRoute(this->position, this->job->origin));
        // add preJob time to stats
        if(status == Const::JOB) app->addPreTimeToStats(this->job->pre->route->getDuration());
    }
    // start the job
    this->job->start();
}

const STATUS Car::updateJob(Job * jobToUpdate, const std::time_t & currentTime) {
    // check if current step has ended
    while(currentTime >= jobToUpdate->timeOfNextStep){
        // update battery
        this->updateBattery(jobToUpdate->route->getStep(jobToUpdate->step).distance, currentTime);
        // update mileage
        this->mileage += jobToUpdate->route->getStep(jobToUpdate->step).distance;
        // if destination is reached
        if(++jobToUpdate->step == jobToUpdate->route->getStepsNumber()){
            // set car position to the end of route
            this->position = jobToUpdate->route->getStep(jobToUpdate->step-1).end;
            this->printPosition();
            return Const::END;
        }
        // if more steps need to be done
        else{
            // set car position to the beginnig of next step
            this->position = jobToUpdate->route->getStep(jobToUpdate->step).start;
            // schedule next update
            jobToUpdate->timeOfNextStep += jobToUpdate->route->getStep(jobToUpdate->step).duration;
            this->printPosition();
        }
    }
    return Const::OK;
}

void Car::update(const std::time_t & currentTime){
    switch (this->status) {
        case Const::FREE:
            this->updateBattery(currentTime);
            break;
        case Const::JOB:
            // update job and check weather it has ended
            switch(this->updateJob(this->job, currentTime)) {
                case Const::OK:
                    // nothing happens
                    break;
                case Const::END:
                    // delete job and change status to free
                    delete this->job;
                    this->job = nullptr;
                    this->setStatus(Const::FREE);
                    break;
            }
            break;
        case Const::PREJOB:
            // update prejob and check weather it has ended
            switch(this->updateJob(this->job->pre, currentTime)) {
                case Const::OK:
                    // nothing happens
                    break;
                case Const::END:
                    // delete prejob and start right job
                    delete this->job->pre;
                    this->job->pre = nullptr;
                    this->job->start();
                    this->setStatus(Const::JOB);
                    break;
            }
            break;
        case Const::TO_BASE:
            // update job and check weather it has ended
            switch(this->updateJob(this->job, currentTime)) {
                case Const::OK:
                    // nothing happens
                    break;
                case Const::END:
                    // delete job and change status to Const::IN_BASE
                    delete this->job;
                    this->job = nullptr;
                    this->setStatus(Const::IN_BASE);
                    break;
            }
            break;
    }

    // process wargings
    for(const STATUS & warning : this->warnigs) switch(warning) {
        case Const::LOW_BATTERY:
        case Const::CRITICAL_BATTERY: 
            if(this->status == Const::FREE) app->sendToClosestBase(this);
            Util::log("car " + std::to_string(this->id) + ":\tgoing to base after " + std::to_string(warning) + " warning");
            break;
    }
}

void Car::charge(const unsigned int & toCharge) {
    this->battery += toCharge;
    if(this->battery > 1000000) this->battery = 1000000;
}

void Car::checkInPort() { this->setStatus(Const::IN_PORT); }

void Car::printPosition() const {
    Util::log("car " + std::to_string(this->id) + "\tstatus: " + std::to_string(this->status)
        + "\tpos: " + std::to_string(this->position.getLat()) + " " + std::to_string(this->position.getLng())
        + "\tbat: " + std::to_string(this->battery));
}

const nlohmann::json Car::getJobJson() const {
    auto json = nlohmann::json();
    json["id"] = this->id;
    json["origin"] = this->job->route->getStartName();
    json["destination"] = this->job->route->getEndName();
    json["duration"] = this->job->route->getDuration();
    json["preDuration"] = (this->job->pre != nullptr) ? this->job->pre->route->getDuration() : 0;
    return json;
}

 void to_json(nlohmann::json & j, const Car & c) {
    j["id"] = c.getId();
    j["status"] = c.getStatus();
    j["pos"]["lat"] = c.getPos().getLat();
    j["pos"]["lng"] = c.getPos().getLng();
    j["battery"] = c.getBattery();
    j["mileage"] = c.getMilleage();
    if(c.getJob() != nullptr){
        j["job"]["duringJob"] = true; 
        if(c.getJob()->pre != nullptr) {
            j["job"]["start"] = c.getJob()->pre->route->getStartName();
            j["job"]["end"] = c.getJob()->pre->route->getEndName();
        }
        else {
            j["job"]["start"] = c.getJob()->route->getStartName();
            j["job"]["end"] = c.getJob()->route->getEndName();
        }
    }
    else j["job"]["duringJob"] = false;
    j["warnings"] = nlohmann::json(c.warnigs);
 }