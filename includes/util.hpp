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

        // required by nlohmann::json while parsing object to JSON
		friend void to_json(nlohmann::json & j, const Position & p);

        static const double getDistance(const Position & A, const Position & B);
};

struct AreaSegment {
    const Position & A;
    const Position & B;

    // y = ax + b
    const double a;
    const double b;

    AreaSegment(const Position & _A, const Position & _B);
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