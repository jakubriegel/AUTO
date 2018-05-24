#pragma once

// stl
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <cmath>
#include <utility>
#include <mutex>
#include <random>

// external
#include "crow_all.h"
#include "json.hpp"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

// create aliases [for convinience]
using STATUS = unsigned int;
using LOCK = std::lock_guard<std::mutex>;

// internal
#include "util.hpp"
#include "Car.hpp"
#include "url.hpp"
#include "AUTO.hpp"

// const values
namespace Const{
    // universal positive status
    const STATUS OK = 1001;
    // universal end status
    const STATUS END = 1002;

    // statuses
    const STATUS FREE = 101;
    const STATUS JOB = 102;
    const STATUS PREJOB = 103;
    const STATUS IN_BASE = 104;
    const STATUS TO_BASE = 105;

    const STATUS CAR_AVAILABLE = 201;
    const STATUS NO_CAR_AVAILABLE = 202;
    const STATUS OUTSIDE_ALLOWED_AREA = 203;

    // warnings
    const STATUS LOW_BATTERY = 501;
    const STATUS CRITICAL_BATTERY = 502;
    const STATUS CHARGING = 503;

    // localization of main base
    const Position BASE = Position(52.403766, 16.9511976); // PUT

    // time consts | in seconds
    const unsigned int MINUTE = 60;
}