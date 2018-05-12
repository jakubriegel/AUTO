#pragma once

using STATUS = const unsigned int;

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

// external
#include "crow_all.h"
#include "json.hpp"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

// internal
#include "util.hpp"
#include "Car.hpp"
#include "url.hpp"
#include "AUTO.hpp"

// const values
namespace Const{
    STATUS FREE = 101;
    STATUS JOB = 102;
    STATUS PREJOB = 103;

    STATUS CAR_AVAILABLE = 201;
    STATUS NO_CAR_AVAILABLE = 202;
    STATUS OUTSIDE_ALLOWED_AREA = 203;

    const Position BASE = Position(52.403766, 16.9511976); // PUT
}