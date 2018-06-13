#pragma once

#include "project.hpp"

class Position {
	private:
		double lat;
		double lng;

	public:
		Position() = default;
			Position(double _lat, double _lng) : lat(_lat), lng(_lng) {};

		// getters for private members
		const double & getLat() const { return lat; };
		const double & getLng() const { return lng; };

		// compare equality operator
		bool operator==(const Position & p2) const { return lat == p2.getLat() && lng == p2.getLng(); };
		bool operator!=(const Position & p2) const { return lat != p2.getLat() || lng != p2.getLng(); };

        // required by nlohmann::json while parsing object to JSON
		friend void to_json(nlohmann::json & j, const Position & p);

		// get distance in meteres between two positions
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

class Area {
	private:
		std::vector<Position> nodes;
		std::vector<AreaSegment> segments;

		double minLat;
		double maxLat;
		double minLng;
		double maxLng;

	public:
		Area() : minLat(90), maxLat(0), minLng(180), maxLng(0) {}

		void addNode(const double & lat, const double & lng);
		void addSegment(const Position & A, const Position & B) { this->segments.emplace_back(A, B); }

		const std::vector<Position> & getNodes() const { return this->nodes; }
		const std::vector<AreaSegment> & getSegments() const { return this->segments; }

		const double & getMinLat() const { return this->minLat; }
		const double & getMaxLat() const { return this->maxLat; }
		const double & getMinLng() const { return this->minLng; }
		const double & getMaxLng() const { return this->maxLng; }

		// required by nlohmann::json while parsing object to JSON
		friend void to_json(nlohmann::json & j, const Area & a);
};

struct Step {
	Position start;
	Position end;

	unsigned int distance; // in meters
	unsigned int duration; // in seconds
	
	Step(Position _s, Position _e, unsigned int _d, unsigned int _t)
		: start(_s), end(_e), distance(_d), duration(_t) {};
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

		const nlohmann::json data;

	public:
		Route(nlohmann::json _data);

		void print(bool extended = false) const;

		// set custom destination name
		void setEndName(const std::string & name) { this->endName = name; };

		// getters to provate members
		const Position & getStart() const { return this->start; };
		const Position & getEnd() const { return this->end; };
		const std::string & getStartName() const { return this->startName; }
		const std::string & getEndName() const { return this->endName; }
		const Step & getStep(const unsigned int & i) const { return this->steps[i]; };
		const unsigned int & getStepsNumber() const { return this->stepsN; };
		const unsigned int & getDuration() const { return this->duration; }

};

struct Job{
	// ride to bo done
	Route * route;
	const Position & origin; // for convinience
	const Position & destination; // for convinience

	std::time_t startTime;
	std::time_t timeOfNextStep;
	unsigned int step;

	// possible ride to origin of this job
	Job * pre;

	Job(Route * _route);
	~Job() { delete this->route, this->pre; }

	void start();
};

// functions used in vaurious cases
namespace Util {
	// convert degrees to radians
	inline const double toRad(const double & deg) { return deg * M_PI / 180; }

	// random numbers generators
	double randDouble(double min, double max);
	unsigned int randUnInt(unsigned int min, unsigned int max);

	// standard logging
	void log(const std::string & msg, const bool & error = false);
}