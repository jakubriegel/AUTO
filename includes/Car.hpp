#pragma once

#include "project.hpp"

// forward declaration of AUTO, for making it possible to use it as class member here
class AUTO;

class Car {
private:
	// number of cars on duty
	static unsigned int inUse;

	// pointer to the application, making it possible to communicate with it in both sides
	AUTO * app;

	// unic id
	unsigned int id;
	// current position
	Position position;
	// current status
	STATUS status;
	// set status and inform the app
	void setStatus(const STATUS & newStatus);
	// time of duty start
	std::time_t startTime;

	// list of warings
	std::vector<STATUS> warnigs;
	// add new warning if it is not already on the list
	void addWarning(const STATUS & warning);

	// current job
	Job * job;
	// update step of the job
	const STATUS updateJob(Job * jobToUpdate, const std::time_t & currentTime);

	// route to the job start
	Job * preJob;

	// battery specs
	// maximal distance to go on full battery | in meters
	static unsigned int range;
	// % of battery used for drive 1m (times 10000)
	static unsigned int perMeter;
	// meters for which to drive enegry is used during 1min of stand-by
	static unsigned int minuteDistace;
	// % of battery letting car drive no more than 50km | times 10000
	static unsigned int lowBattery;
	// % of battery letting car drive no more than 15km
	static unsigned int criticalBattery;
	// current battery level | 0-100% (times 10000)
	unsigned int battery;
	// total mileage for current duty | in meters
	unsigned int mileage;
	
	// subtrack stand-by battery usage and add necessary warings
	std::time_t timeOfNextBatteryUpdate;
	const STATUS updateBattery();
	const STATUS updateBattery(const std::time_t & currentTime);
	// subtrack used % and add perform updateBattery()
	const STATUS updateBattery(const unsigned int & driven, const std::time_t & currentTime);


public:
	// calculate range and per meter usage of battery | parameter in km
	static void calcBatteryUsage(unsigned int _range, unsigned int _minute); 
	static void init();
	
	Car(AUTO * _app, const Position & _position);

	// assign new job
	void addJob(Route * route);
	// assign new job with predefined status | ex. used while sending car to the base
	void addJob(Route * route, const STATUS & status, const STATUS & statusPre);
	// update car data
	void update(const std::time_t & currentTime);
	// add battery power | % times 10000
	void charge(const unsigned int & toCharge);
	// check in port
	void checkInPort();
	
	void printPosition() const;

	// getters to private members
	const unsigned int & getId() const { return this->id; };
	const Position & getPos() const { return this->position; };
	const STATUS & getStatus() const { return this->status; };
	const unsigned int & getBattery() const { return this->battery; }
	const unsigned int & getMilleage() const { return this->mileage; }
	Job * const getJob() const { return this->job; }

	// get current job details
	const nlohmann::json getJobJson() const;


	
	// convert to nlohmann::json
	friend void to_json(nlohmann::json & j, const Car & c);
};