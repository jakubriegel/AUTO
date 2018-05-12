#pragma once

#include "project.hpp"

// forward declaration of AUTO, for making it possible to use it as class member here
class AUTO;

class Car {
private:
	static unsigned int inUse; // number of cars on duty

	AUTO * app;

	unsigned int id; // unical id
	Position position; // current position
	unsigned int status;
	// set status and inform the app
	void setStatus(unsigned int newStatus);

	Job * job;
	Job * preJob;

	// driving specs
	static unsigned short range;
	static unsigned char perMeter; // % of battery used for drive 1km (times 10000)
	unsigned short battery; // 0-100% (times 10000)
	unsigned int mileage; // total in meters

	static void countBatteryUsage(unsigned short _range); // parameter in km

	// void goToPort();
	// void endDuty();

public:
	static void init();
	Car(AUTO * _app);

	void addJob(Route * route);
	void update(const std::time_t & currentTime);
	
	void printPosition();

	const unsigned int & getId() const { return id; };
	const Position & getPos() const { return position; };
	const unsigned int & getStatus() const { return status; };

};