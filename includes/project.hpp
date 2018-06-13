#pragma once

// for using math consts
#define _USE_MATH_DEFINES

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
#include <limits>
#include <cfloat>
#include <typeinfo>

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
#include "Const.hpp"
#include "util.hpp"
#include "Base.hpp"
#include "Car.hpp"
#include "url.hpp"
#include "AUTO.hpp"

