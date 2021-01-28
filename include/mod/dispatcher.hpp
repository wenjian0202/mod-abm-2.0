#pragma once

#include "types.hpp"

template<typename RouterFunc>
int assign_trips_to_vehicles(const int trip, const int vehicle, RouterFunc& func)
{
    Pos o;
    Pos d;
    return func(o, d);
}