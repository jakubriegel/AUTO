#pragma once

#include "project.hpp"

// const values
class Const final {
    // values read from config.json
    private:
        // simulator consts
        static unsigned int SIMULATOR_INTERVAL;

        // orders generator consts
        static unsigned int GENERATOR_DELAY;
        static unsigned int GENERATOR_INTERVAL_MIN;
        static unsigned int GENERATOR_INTERVAL_MAX;
        static double GENERATOR_MAIN_PROB;

        // Google Maps API key
        static std::string MAPS_API_KEY;

        // make class abstract
        virtual void pureVirtualMethod() = 0;
    
    // predefined values and getters for dynamic values
    public: 
        // universal positive status
        static constexpr STATUS OK = 1001;
        // universal negative status
        static constexpr STATUS ERROR = 1002;
        // universal end status
        static constexpr STATUS END = 1003;

        // statuses
        static constexpr STATUS FREE = 101;
        static constexpr STATUS JOB = 102;
        static constexpr STATUS PREJOB = 103;
        static constexpr STATUS IN_BASE = 104;
        static constexpr STATUS TO_BASE = 105;
        static constexpr STATUS IN_PORT = 106;
        static constexpr STATUS CHARGING = 107;

        static constexpr STATUS CAR_AVAILABLE = 201;
        static constexpr STATUS NO_CAR_AVAILABLE = 202;
        static constexpr STATUS OUTSIDE_ALLOWED_AREA = 203;

        // warnings
        static constexpr STATUS LOW_BATTERY = 501;
        static constexpr STATUS CRITICAL_BATTERY = 502;

        // time consts | in seconds
        static constexpr unsigned int MINUTE = 60;

        // Earth radius used for calculating distance | in m
        static constexpr double EARTH_RADIUS = 6371000.0;    

        // getters and setters for dynamic values
        static void setSimulatorInterval(const unsigned int & val) { Const::SIMULATOR_INTERVAL = val; }
        static const unsigned int & getSimulatorInterval() { return Const::SIMULATOR_INTERVAL; }

        static void setGeneratorDelay(const unsigned int & val) { Const::GENERATOR_DELAY = val; }
        static const unsigned int & getGeneratorDelay() { return Const::GENERATOR_DELAY; }

        static void setGeneratorIntervalMin(const unsigned int & val) { Const::GENERATOR_INTERVAL_MIN = val; }
        static const unsigned int & getGeneratorIntervalMin() { return Const::GENERATOR_INTERVAL_MIN; }

        static void setGeneratorIntervalMax(const unsigned int & val) { Const::GENERATOR_INTERVAL_MAX = val; }
        static const unsigned int & getGeneratorIntervalMax() { return Const::GENERATOR_INTERVAL_MAX; }

        static void setGeneratorMainProb(const double & val) { Const::GENERATOR_MAIN_PROB = val; }
        static const double & getGeneratorMainProb() { return Const::GENERATOR_MAIN_PROB; }

        static void setMapsApiKey(const std::string & val) { Const::MAPS_API_KEY = std::move(val); }
        static const std::string & getMapsApiKey() { return Const::MAPS_API_KEY; }

};