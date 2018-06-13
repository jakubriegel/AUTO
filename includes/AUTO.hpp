#pragma once

#include "project.hpp"

class AUTO{
private:
    // area
    std::vector<Area> area;
    std::mutex areaMutex;
    Position referencePoint;

    // cars
    std::vector<Car*> activeCars;
    std::vector<Car*> freeCars;
    std::vector<Car*> busyCars;
    std::mutex carsMutex;
    // create new car and append it to the list of cars
    Car * addCar();
    unsigned int activeCarsNo;
    unsigned int freeCarsNo;
    unsigned int busyCarsNo;
    void setCarAsFree(Car * car);
    void setCarAsBusy(Car * car);

    // bases
    Port * port;
    std::vector<Base> bases;
    std::mutex basesMutex;

    // statistics
    // rides taken
    unsigned int takenRides;
    // average preJob time
    unsigned int averagePreTime;

    // thread function for server
    void server();
    // thread function for vechicles simulator
    void simulator();
    // thread function for orders simulator
    void orders();

    // initialize the app
    void init(const std::string & configFile);
    // check if positon is in allowed area
    const bool inArea(const Position & A) const;
    // looks for closest availble car
    Car * const getCar(const Position & A, const STATUS & exluded = 0, const unsigned int & radius = UINT_MAX) const;
    // check if any car is available at A
    const STATUS isCarAvailable(const Position & A) const;
    // get directions and assing car for route
    Car * const requestRoute(const Position & A, const Position & B);
    // make car go to the selected base
    void sendToBase(Car * car, Base * base);    

public:
    AUTO(const std::string & configFile);

    // update car vectors after change of one cars' status
    void updateCarStatus(Car * car, const STATUS & old);
    // respond to the bases' car request | get directions and assing car
    Car * requestToBase(Base * base, const unsigned int & radius = UINT_MAX);
    // make car go to the port
    void sendToPort(Car * car);
    // make car go to the closest base with free slots
    void sendToClosestBase(Car * car);
    // add stats
    void addRideToStats() { this->takenRides++; }
    void addPreTimeToStats(const unsigned int & time) { this->averagePreTime = ((this->averagePreTime * (this->takenRides-1)) + time) / this->takenRides; }
};