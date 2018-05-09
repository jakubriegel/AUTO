#pragma once

#include "project.hpp"

class AUTO{
private:
    void readConfig(const std::string & configFile);

    std::vector<std::vector<Position>> area;

    // containers for cars
    std::vector<Car*> activeCars;
    std::vector<Car*> freeCars;
    std::vector<Car*> busyCars;
    unsigned int activeCarsNo;
    unsigned int freeCarsNo;
    unsigned int busyCarsNo;
    Car* addCar();
    
    void setCarAsFree(Car * car);
    void setCarAsBusy(Car * car);

    std::vector<Route*> routes;

    void server();
    void simulator();

    // check if any car is available at A [for now: check if at least one car is free]
    bool isCarAvailable(const Position & A) const;
    // get directions and assing car for route
    const unsigned int requestRoute(const Position & A, const Position & B);

public:
    AUTO(const std::string & configFile);

    void updateCarStatus(Car * car, unsigned int old);
};