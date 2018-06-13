#include "../project.hpp"

// members declared again here for assigning them memory during application start
const STATUS Const::OK;
const STATUS Const::ERROR;
const STATUS Const::END;
const STATUS Const::FREE;
const STATUS Const::JOB;
const STATUS Const::PREJOB;
const STATUS Const::IN_BASE;
const STATUS Const::TO_BASE;
const STATUS Const::IN_PORT;
const STATUS Const::CHARGING;
const STATUS Const::CAR_AVAILABLE;
const STATUS Const::NO_CAR_AVAILABLE;
const STATUS Const::OUTSIDE_ALLOWED_AREA;
const STATUS Const::LOW_BATTERY;
const STATUS Const::CRITICAL_BATTERY;
const unsigned int Const::MINUTE;
constexpr double Const::EARTH_RADIUS;
unsigned int Const::SIMULATOR_INTERVAL;
unsigned int Const::GENERATOR_DELAY;
unsigned int Const::GENERATOR_INTERVAL_MIN;
unsigned int Const::GENERATOR_INTERVAL_MAX;
double Const::GENERATOR_MAIN_PROB;
std::string Const::MAPS_API_KEY;