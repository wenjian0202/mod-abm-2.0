#pragma once

#include "types.hpp"

/// @brief Stateful functor that finds the shortest route for an O/D pair on request. 
class Router
{
public:
    Router() {};

    int operator() (const Pos& origin, const Pos& destination);
private:
    int count = 0;
};



