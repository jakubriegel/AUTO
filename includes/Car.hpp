#pragma once

#include "project.hpp"

class Position {
	private:
		double lat;
		double lng;

	public:
		Position() = default;
			Position(double _lat, double _lng) : lat(_lat), lng(_lng) {};

		const double & getLat() const { return lat; };
		const double & getLng() const { return lng; };

		bool operator==(const Position & p2) const { return lat == p2.getLat() && lng == p2.getLng(); };

		friend void to_json(nlohmann::json & j, const Position & p) {
			j = nlohmann::json{{"lat", p.getLat()}, {"lng", p.getLng()}};
		}
};

struct Step {
	Position start;
	Position end;

	unsigned int duration; // in seconds
	
	Step(Position _s, Position _e, unsigned int _t)
		: start(_s), end(_e), duration(_t) {};
};

class Route {
	private:
		unsigned int distance; // in meters
		unsigned int duration; // in seconds

		Position start;
		Position end;
		std::string startName;
		std::string endName;
		std::vector<Step> steps;
		unsigned int stepsN;

		nlohmann::json data;

	public:
		Route(nlohmann::json _data);

		void print(bool extended = false) const;

		const Position & getStart() const { return start; };
		const Position & getEnd() const { return end; };
		const Step & getStep(unsigned int i) const { return steps[i]; };
		const unsigned int & getStepsNumber() const { return stepsN; };

};

struct Job{
	Route * route;
	const Position & origin; // for convinience
	const Position & destination; // for convinience

	std::time_t startTime;
	std::time_t timeOfNextStep;
	unsigned int step;

	Job(Route * _route);
	~Job() { delete route; }

	void start();
};

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