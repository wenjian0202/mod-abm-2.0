/// \author Jian Wen
/// \date 2021/02/01

#pragma once

#include "config.hpp"
#include "types.hpp"

#include <queue>

/// \brief The agent-based modeling platform that simulates the mobility-on-demand systems.
template <typename RouterFunc, typename DemandGeneratorFunc>
class Platform
{
public:
    /// \brief Constructor.
    explicit Platform(PlatformConfig _platform_config, RouterFunc _router_func, DemandGeneratorFunc _demand_generator_func);

    void run_simulation();

    // void invoke_demand_generator()
    // {
    //     // Pos origin{113.93593149478123, 22.312648328005512};
    //     // Pos destination{114.13602296340699, 22.28328541732128};

    //     // auto ret = router_func_(origin, destination);

    //     // demand_generator_func_(100);
    // }

private:
    void run_cycle();

    void dispatch();

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

    /// \brief The queue of iterators pointing to trips to be dispatched.
    std::queue<const std::vector<Trip>::iterator> pending_trips_ = {};
};

// Implementation is put in a separate file for clarity and maintainability.
#include "platform_impl.hpp"
