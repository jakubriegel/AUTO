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
    std::cout << "Car::range = " << Car::range 
        << "\nCar::lowBattery = " << Car::lowBattery
        << "\nCar::criticalBattery = " << Car::criticalBattery << "\n";
}

void Car::init(){

}

// constructor of Car
Car::Car(AUTO * _app)
    : app(_app), id(++inUse), position(Const::BASE),
    job(nullptr), preJob(nullptr), battery(1000000), 
    mileage(0), timeOfNextBatteryUpdate(std::time(0) + Const::MINUTE) {
        this->setStatus(Const::FREE);
};


void Car::setStatus(const STATUS & newStatus) {
    const STATUS old = this->status;
    this->status = newStatus;
    // let the app know about change
    this->app->updateCarStatus(this, old);
}

void Car::addWarning(const STATUS & warning) {
    // check if warnig is not already on the list and then add it
    if(this->warnigs.empty()) this->warnigs.push_back(warning);
    else if(std::find(this->warnigs.begin(), this->warnigs.end(), warning) == this->warnigs.end()) this->warnigs.push_back(warning);
}

const STATUS Car::updateBattery() {
    // get current time and call actual function
    return this->updateBattery(std::time(0));
}

const STATUS Car::updateBattery(const std::time_t & currentTime) {
    // check if it is time for update
    if(currentTime >= this->timeOfNextBatteryUpdate) {
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
    // assign job
    this->job = new Job(route);
    // if car is in start point set status to JOB
    if(this->position == this->job->origin) this->setStatus(Const::JOB);
    // if not create prejob and set appropiate status
    else{
        this->setStatus(Const::PREJOB);
        this->job->pre = new Job(url::getRoute(this->position, this->job->origin));
    }
    // start the job
    this->job->start();
}

const STATUS Car::updateJob(Job * jobToUpdate, const std::time_t & currentTime) {
    // check if current step has ended
    if(currentTime >= jobToUpdate->timeOfNextStep){
        // update battery
        this->updateBattery(jobToUpdate->route->getStep(jobToUpdate->step).distance, currentTime);
        // update mileage
        this->mileage += jobToUpdate->route->getStep(jobToUpdate->step).distance;
        // if destination is reached
        if(++jobToUpdate->step == jobToUpdate->route->getStepsNumber()){
            // set car position to the end of route
            this->position = jobToUpdate->route->getStep(jobToUpdate->step-1).end;
            return Const::END;
        }
        // if more steps need to be done
        else{
            // set car position to the beginnig of next step
            this->position = jobToUpdate->route->getStep(jobToUpdate->step).start;
            // schedule next update
            jobToUpdate->timeOfNextStep += jobToUpdate->route->getStep(jobToUpdate->step).duration;
            return Const::OK;
        }

        this->printPosition();
    }
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
    }

    // check for warnings [to be implemented]
    if(!this->warnigs.empty()) {
        std::cout << "warn ";
    }
}

void Car::printPosition() const{
    std::cout << "car " << this->id << " " << this->status << " position:\t" << this->position.getLat() << "\t" << this->position.getLng() << " bat: " << this->battery << "\n";
}