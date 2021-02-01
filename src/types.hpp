/// \author Jian Wen
/// \date 2021/01/29

#pragma once

#include <osrm/json_container.hpp>

#include <yaml-cpp/yaml.h>

#include <string>

//////////////////////////////////////////////////////////////////////////////////////////////////
/// Basic Types
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
/// Config Types
//////////////////////////////////////////////////////////////////////////////////////////////////

/// \brief Config that describes the simulated area.
struct AreaConfig
{
    double lon_min = 0.0; // max longitude accepted
    double lon_max = 0.0; // min longitude accepted
    double lat_min = 0.0; // max latitude accepted
    double lat_max = 0.0; // min latitude accepted
};

/// \brief Config that describes the fleet.
struct FleetConfig
{
    size_t fleet_size = 10;  // fleet size
    size_t veh_capacity = 2; // vehicle capacity, 1 = non-shared, >2 is shared
};

/// \brief Config that describes the requests.
struct RequestConfig
{
    double walkaway_time_s = 600; // the time after which the requesting traveller will cancel the request
};

/// \brief Config that describes the target MoD system.
struct MoDSystemConfig
{
    FleetConfig fleet_config;
    RequestConfig request_config;
};

/// \brief Config that describes the simulation statistics.
struct SimulationConfig
{
    double cycle_s = 30.0;               // the cycle every x second the platform dispatches the requests in batch
    double simulation_duration_s = 3600; // the main period during which the simulated data are analyzed
    double warmup_duration_s = 1800;     // the period before the main sim for system to build up states
    double winddown_duration_s = 1800;   // the period after the main sim for system to close trips
};

/// \brief The set of config parameters for the simulation platform.
struct PlatformConfig
{
    AreaConfig area_config;
    MoDSystemConfig mod_system_config;
    SimulationConfig simulation_config;
    std::string path_to_osrm_data; // path to the base osrm map data
};

/// \brief Convert the loaded yaml platform config into the c++ data struct.
PlatformConfig convert_yaml_to_platform_config(const YAML::Node &platform_config_yaml);