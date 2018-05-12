#pragma once

#include "project.hpp"

namespace url {
    Route * getRoute(const Position & a, const Position & b);
    const Position getCoords(const std::string & location);
}