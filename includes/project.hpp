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

// external
#include "crow_all.h"
#include "json.hpp"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

// internal
#include "Car.hpp"
#include "url.hpp"
#include "AUTO.hpp"

// const values
namespace Const{
    const unsigned int STATUS_FREE = 101;
    const unsigned int STATUS_JOB = 102;
    const unsigned int STATUS_PREJOB = 103;

    const Position BASE = Position(52.403766, 16.9511976); // PUT
}