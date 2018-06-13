#include "../project.hpp"

// static members of Base
unsigned int Base::inUse = 0;
unsigned int Base::chargeInterval = 0;
unsigned int Base::chargeAmmount = 0;

void Base::setCharging(const unsigned int & _chargeInterval, const unsigned int & _chargeAmmount) {
    Base::chargeInterval = _chargeInterval;
    Base::chargeAmmount = _chargeAmmount;
}

// constructor of Base
Base::Base(AUTO * _app, std::string _name, Position _position, unsigned int _radius, unsigned int _slotsNo)
     : app(_app), id(++Base::inUse), name(std::move(_name)), position(_position), 
     radius(_radius), slotsNo(_slotsNo), docked(0), reservedNo(0) {
         Util::log("base " + std::to_string(this->id) + " " + this->name 
            + "\tcapacity: " + std::to_string(this->slotsNo) + " radius: " + std::to_string(this->radius));
     };

const STATUS Base::dock(Car * car) {
    if(this->docked < this->slotsNo){
        this->docked++;
        this->slots.push_back(std::make_pair(car, std::time(0) + Base::chargeInterval));

        Util::log("car " + std::to_string(car->getId()) + " docked in " + this->name);
        return Const::OK;
    }
    else return Const::ERROR;
}

const STATUS Base::undock(Car * car) {
    auto comp = [&car](const std::pair<Car*, std::time_t> & a) {
        if(a.first->getId() == car->getId()) return true;
        return false;
    };
    auto carIt = std::find_if(this->slots.begin(), this->slots.end(), comp);
    if(carIt != this->slots.end()){
        this->docked--;
        this->slots.erase(carIt);

        return Const::OK;
    }
    else return Const::ERROR;
}

Car * Base::requestCar() {
    return this->app->requestToBase(this);
}

void Base::update(const std::time_t & currentTime) {
    // dock arrived cars
    auto con = this->reservedSlots.end();
    for(auto i = this->reservedSlots.begin(); i != con; i++) {
        // check if the car has arrived
        if((*i)->getStatus() == Const::IN_BASE){
            this->dock(*i);
            this->reservedSlots.erase(i);
            this->reservedNo--;
            i = --this->reservedSlots.begin();
            con = this->reservedSlots.end();
        }
    }

    // charge cars
    for(auto car : this->slots) if(car.second <= currentTime) {
        // charge car
        car.first->charge(Base::chargeAmmount);
        // schedule next charge
        car.second += Base::chargeInterval;
    }

    // undock departed cars
    auto con2 = this->slots.end();
    for(auto i = this->slots.begin(); i != con2; i++) {
        // check if the car has departed
        if((*i).first->getStatus() != Const::IN_BASE){
            this->undock((*i).first);
            i = --this->slots.begin();
            con2 = this->slots.end();
        }
    }

    // look for cars in the radius
    while(true) {
        auto car = this->app->requestToBase(this, radius);
        // no point of further iterations if no car is in radius area
        if(car == nullptr) break;
        // if there are free spots in th base reserve one for the car
        if(this->getFreeSlots() > 0){
            Util::log("base " + this->name + "\tgetting car " + std::to_string(car->getId()) + " from radius");
            this->reservedSlots.push_back(car);
            this->reservedNo++;
        }
        // in other case send the car to the port
        else {
            Util::log("base " + this->name + " is sending car " + std::to_string(car->getId()) + " to the port");
            app->sendToPort(car);
        }
        
    }

    // refill docks
    while(this->docked + this->reservedNo < this->slotsNo / 2) {
        auto car = this->app->requestToBase(this);
        if(car != nullptr){
            Util::log("base " + this->name + "\tgetting car " + std::to_string(car->getId()) + " by refil");
            this->reservedSlots.push_back(car);
            this->reservedNo++;
        }
        // no point of further iterations if no car is available
        else break;
    }
}

void to_json(nlohmann::json & j, const Base & b) {
    j["id"] = b.getId();
    j["name"] = b.getName();
    j["slots"] = b.getSlotsNo();
    j["free"] = b.getFreeSlots();
    j["pos"] = nlohmann::json(b.getPosition());
    j["cars"]["docked"] = nlohmann::json::array();
    for(Car * car : b.getDocked()) j["cars"]["docked"].push_back(car->getId());
    j["cars"]["reserved"] = nlohmann::json::array();
    for(Car * car : b.getReserved()) j["cars"]["reserved"].push_back(car->getId());
    j["radius"] = b.getRadius();
}

// Port constructor
Port::Port(AUTO * _app, std::string _name, Position _position, unsigned int _radius, unsigned int _slotsNo) 
    : Base(_app, _name, _position, _radius, _slotsNo) {

}

void Port::update(const std::time_t & currentTime) {
    /*Base::update(currentTime);
    for(auto car : this->slots) if(car.first->getStatus() != Const::IN_PORT) car.first->checkInPort();*/

    // dock arrived cars
    auto con = this->reservedSlots.end();
    for(auto i = this->reservedSlots.begin(); i != con; i++) {
        // check if the car has arrived
        if((*i)->getStatus() == Const::IN_BASE){
            this->dock(*i);
            (*i)->checkInPort();
            this->reservedSlots.erase(i);
            this->reservedNo--;
            i = --this->reservedSlots.begin();
            con = this->reservedSlots.end();
        }
    }

    // charge cars
    for(auto car : this->slots) if(car.second <= currentTime) {
        // charge car
        car.first->charge(Base::chargeAmmount);
        // schedule next charge
        car.second += Base::chargeInterval;
    }

    // undock departed cars
    auto con2 = this->slots.end();
    for(auto i = this->slots.begin(); i != con2; i++) {
        // check if the car has departed
        if((*i).first->getStatus() != Const::IN_PORT){
            this->undock((*i).first);
            i = --this->slots.begin();
            con2 = this->slots.end();
        }
    }

    // look for cars in the radius
    while(true) {
        auto car = this->app->requestToBase(this, radius);
        // no point of further iterations if no car is in radius area
        if(car == nullptr) break;
        // if there are free spots in th base reserve one for the car
        if(this->getFreeSlots() > 0){
            Util::log("base " + this->name + " getting car " + std::to_string(car->getId()) + " from radius");
            this->reservedSlots.push_back(car);
            this->reservedNo++;
        }
        // in other case send the car to the port
        else {
            Util::log("base " + this->name + " is sending car " + std::to_string(car->getId()) + " to the port");
            app->sendToPort(car);
        }
        
    }

}

void Port::reserveSlot(Car * car) {
    Util::log(this->name + " reserving car " + std::to_string(car->getId()));
    this->reservedSlots.push_back(car);
    this->reservedNo++;
}