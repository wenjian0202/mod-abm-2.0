#include <mod/router.hpp>
#include <mod/dispatcher.hpp>

#include <fmt/format.h>

#include <iostream>

int main()
{
    Router router;

    auto dispatcher = [&router](const int trip, const int vehicle){ return assign_trips_to_vehicles(trip, vehicle, router); };

    fmt::print("first: {}, second: {}, thrid: {}\n", dispatcher(0, 0), dispatcher(0, 0), dispatcher(0, 0));

    return 0;
}
