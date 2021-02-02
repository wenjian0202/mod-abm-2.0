/// \author Jian Wen
/// \date 2021/01/29

#pragma once

#include <osrm/json_container.hpp>

#include <yaml-cpp/yaml.h>

#include <string>

//////////////////////////////////////////////////////////////////////////////////////////////////
/// Geo Types
//////////////////////////////////////////////////////////////////////////////////////////////////

/// \brief Position encoded in longitude/latitude.
/// \details lon in [-180, 180), lat in [-90, 90]. Undefined behaviors if out of bound.
struct Pos
{
    double lon = 0.0;
    double lat = 0.0;
};

/// \brief Step of route consisting of distance, duration as well as a vector of continuous positions.
struct Step
{
    double distance_m = 0.0;
    double duration_s = 0.0;
    std::vector<Pos> poses;
};

/// \brief Leg of route consisting of total distance, total duration as well as a vector of steps.
struct Leg
{
    double distance_m = 0.0;
    double duration_s = 0.0;
    std::vector<Step> steps;
};

/// \brief Route consisting of total distance, total duration as well as a vector of legs.
struct Route
{
    double distance_m = 0.0;
    double duration_s = 0.0;
    std::vector<Leg> legs;
};

/// \brief Convert the json route data into the c++ data struct.
Route convert_json_to_route(osrm::json::Object route_json);

/// \brief The status of the routing response.
enum RoutingStatus
{
    UNDEFINED, // uninitialized value
    OK,        // good route
    EMPTY,     // empty route
    ERROR      // error
};

/// \brief The response from the routing engine.
struct RoutingResponse
{
    RoutingStatus status = RoutingStatus::UNDEFINED;
    std::string message = "";
    Route route;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// Trip Types
//////////////////////////////////////////////////////////////////////////////////////////////////

/// \brief A single demand OD with accumulated probability.
/// \details The accumulated probability of a OD is a double between (0, 1]. We use these to form
/// a list of ODs with increasing probs to be able to generate a random request based on each OD's
/// trip intensity. For example, the first OD has a probability of 0.1 among all trips and the
/// accumulated prob will be 0.1. The second OD has a probability of 0.1 and the accumulated prob
/// will be 0.2 (0.1 + 0.1). The third OD has a probability of 0.05 and the accumulated prob in
/// this case will be 0.25 (0.2 + 0.05). To generate a trip, we generate a random number between
/// 0 and 1. If the random number falls into [0.1, 0.2), the origin/destination of the generated
/// trip will be based on the second OD.
struct DemandOD
{
    Pos origin;
    Pos destination;
    double accumulated_prob;
};

/// \brief The trip request represented by an OD pair and a request time.
struct Request
{
    Pos origin;
    Pos destination;
    double request_time_s = 0.0;
};