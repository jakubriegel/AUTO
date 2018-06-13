#pragma once

#include "project.hpp"

// forward declaration of Car, for making it possible to use it as class member here
class AUTO;
class Car;

class Base {
	protected:
		// pointer to the application, making it possible to communicate with it in both sides
		AUTO * app;

		// number of bases
		static unsigned int inUse;

		// id of the base
		const unsigned int id;
		// name of the base
		const std::string name;
		// position of the base
		const Position position;
		// radius of attracting cars
		const unsigned int radius;
		// number of slots in the base
		const unsigned int slotsNo;
		// cars docked in slots of the base | stored with time of next charge
		std::vector<std::pair<Car*, std::time_t>> slots;
		// number of cars docked in the base
		unsigned int docked;
		// number of reserved slots in the base
		unsigned int reservedNo;
		// cars that have slots reserved in the base
		std::vector<Car*> reservedSlots;

		// interval of charging
		static unsigned int chargeInterval;
		// % of battery given to car durring each charge | times 10000
		static unsigned int chargeAmmount;

		// check if any free cars are in the radius
		void checkRadius();
		// request car to dock in the base
		Car * requestCar();

	public:
		// set up charging parameters
		static void setCharging(const unsigned int & _chargeInterval, const unsigned int & _chargeAmmount);

		Base(AUTO * _app, std::string _name, Position _position, unsigned int _radius, unsigned int _slotsNo);

		// dock selected car in the base
		const STATUS dock(Car * car);
		// undock selected car from the base
		const STATUS undock(Car * car);
		
		// update the base
		virtual void update(const std::time_t & currentTime);

		// convert selected base to nlohmann::json
		friend void to_json(nlohmann::json & j, const Base & b);

		// getters to private members
		const unsigned int & getId() const { return this->id; }
		const std::string & getName() const { return this->name; }
		const Position & getPosition() const { return this->position; }
		const unsigned int & getRadius() const { return this->radius; }
		const unsigned int & getSlotsNo() const { return this->slotsNo; }
		const std::vector<Car*> getDocked() const { 
			std::vector<Car*> cars;
			for(const auto & i : this->slots) cars.push_back(i.first);
			return cars;
		}
		const std::vector<Car*> & getReserved() const { return this->reservedSlots; }
		// get number of available slots
		const unsigned int getFreeSlots() const { return this->slotsNo - this->docked - this->reservedNo; }
};

class Port : public  Base {

    public:
        Port(AUTO * _app, std::string _name, Position _position, unsigned int _radius, unsigned int _slotsNo);

        virtual void update(const std::time_t & currentTime) override;
		void reserveSlot(Car * car);
};