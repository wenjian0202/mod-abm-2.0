/// \author Jian Wen
/// \date 2021/02/01

#pragma once

#include "config.hpp"
#include "types.hpp"
#include "vehicle.hpp"

#include <fstream>
#include <chrono>
#include <ctime>

/// \brief The agent-based modeling platform that simulates the mobility-on-demand system.
template <typename RouterFunc, typename DemandGeneratorFunc>
class Platform
{
public:
    /// \brief Constructor.
    explicit Platform(
        PlatformConfig _platform_config,
        RouterFunc _router_func,
        DemandGeneratorFunc _demand_generator_func);

    /// \brief Destructor.
    ~Platform();

    /// \brief Delete the other constructors. Rule of five.
    Platform(const Platform &other) = delete;
    Platform(Platform &&other) = delete;
    Platform &operator=(const Platform &other) = delete;
    Platform &operator=(Platform &&other) = delete;

    /// \brief Run simulation. The master function that manages the entire simulation process.
    void run_simulation();

private:
    /// \brief Run simulation for one cycle. Invoked repetetively by run_simulation().
    void run_cycle();

    /// \brief Advance all vehicles for given time and move forward the system time. Invoked by run_cycle().
    void advance_vehicles(double time_s);

    /// \brief Dispatch vehicles to serve pending trips. Invoked at the end of the cycle by run_cycle().
    void dispatch(const std::vector<size_t> &pending_trip_ids);

    /// \brief Write the data of the current system state to datalog.
    void write_state_to_datalog();

    /// \brief Write the data of all trips to datalog after the simulation.
    void write_trips_to_datalog();

    /// \brief Create the report based on the statistical analysis using the simulated data.
    void create_report();

    /// \brief The set of config parameters for the simulation platform.
    PlatformConfig platform_config_;

    /// \brief The router functor which finds the shortest route for an O/D pair on request.
    RouterFunc router_func_;

    /// \brief The demand generator functor which generates trip requests in each cycle.
    DemandGeneratorFunc demand_generator_func_;

    /// \brief The system time starting from 0.0.
    double system_time_s_ = 0.0;

    /// \brief The vector of trips created during the entire simulation process.
    std::vector<Trip> trips_ = {};

    /// \brief The vector of trips created during the entire simulation process.
    std::vector<Vehicle> vehicles_ = {};

    /// \brief The ofstream that outputs to the datalog.
    std::ofstream fout_datalog;

    /// \brief The total runtime of simulation.
    std::chrono::duration<double> runtime_;
};

// Implementation is put in a separate file for clarity and maintainability.
#include "platform_impl.hpp"
