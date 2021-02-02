/// \author Jian Wen
/// \date 2021/02/01

#pragma once

#include "types.hpp"

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

/// \brief Load yaml platform config and convert into the c++ data struct.
PlatformConfig load_platform_config(const std::string &path_to_platform_config);