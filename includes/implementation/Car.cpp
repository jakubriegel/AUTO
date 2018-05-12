#include "../project.hpp"

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
        this->setStatus(Const::FREE);
};

void Car::addJob(Route * route){
    this->job = new Job(route);
    if(this->position == this->job->origin){
        this->setStatus(Const::JOB);
        this->job->start();
    }
    else{
        this->setStatus(Const::PREJOB);
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
        case Const::FREE:
        break;
        case Const::JOB:
            if(currentTime >= this->job->timeOfNextStep){
                printPosition();
                if(++this->job->step == this->job->route->getStepsNumber()){
                    this->position = this->job->route->getStep(job->step-1).end;
                    std::cout << "car " << this->id << " arrived at the destination\n";
                    this->setStatus(Const::FREE);
                    delete this->job;
                    this->job = nullptr;
                }
                else{
                    this->position = this->job->route->getStep(this->job->step).start;
                    this->job->timeOfNextStep += this->job->route->getStep(this->job->step).duration;
                }
            }
        break;
        case Const::PREJOB:
            if(currentTime >= this->preJob->timeOfNextStep){
                this->printPosition();
                if(++this->preJob->step == this->preJob->route->getStepsNumber()){
                    this->position = this->preJob->route->getStep(preJob->step-1).end;
                    std::cout << "car " << this->id << " ready to start job\n";
                    this->setStatus(Const::JOB);
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