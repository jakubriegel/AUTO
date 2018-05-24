#pragma once

#include "project.hpp"

class AUTO{
private:
    void readConfig(const std::string & configFile);

    // area
    std::vector<Area> area;
    std::mutex areaMutex;
    Position referencePoint;

    // cars
    std::vector<Car*> activeCars;
    std::vector<Car*> freeCars;
    std::vector<Car*> busyCars;
    std::mutex carsMutex;
    unsigned int activeCarsNo;
    unsigned int freeCarsNo;
    unsigned int busyCarsNo;
    Car* addCar();
    
    void setCarAsFree(Car * car);
    void setCarAsBusy(Car * car);

    std::vector<Route*> routes;

    // thread function for server
    void server();
    // thread function for vechicles simulator
    void simulator();
    // thread function for orders simulator
    void orders();

    // check if positon is in allowed area
    const bool inArea(const Position & A) const;
    // looks for closest availble car
    Car * getCar(const Position & A) const;
    // check if any car is available at A
    const STATUS isCarAvailable(const Position & A) const;
    // get directions and assing car for route
    const unsigned int requestRoute(const Position & A, const Position & B);

public:
    AUTO(const std::string & configFile);

    void updateCarStatus(Car * car, const STATUS & old);
};