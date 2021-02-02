/// \author Jian Wen
/// \date 2021/02/01

#pragma once

#include "config.hpp"
#include "types.hpp"

/// \brief The agent-based modeling platform that simulates the mobility-on-demand systems.
template <typename RouterFunc, typename DemandGeneratorFunc>
class Platform
{
public:
    /// \brief Constructor.
    explicit Platform(PlatformConfig _platform_config, RouterFunc _router_func, DemandGeneratorFunc _demand_generator_func)
        : platform_config_(std::move(_platform_config)), router_func_(std::move(_router_func)), demand_generator_func_(std::move(_demand_generator_func))
    {
    }

    void invoke_router()
    {
        Pos origin{113.93593149478123, 22.312648328005512};
        Pos destination{114.13602296340699, 22.28328541732128};

        auto ret = router_func_(origin, destination);
    };

    void invoke_demand_generator()
    {
        demand_generator_func_();
    }

private:
    /// \brief The set of config parameters for the simulation platform.
    PlatformConfig platform_config_;

    /// \brief The router functor which finds the shortest route for an O/D pair on request.
    RouterFunc router_func_;

    /// \brief The demand generator functor which generates trip requests in each cycle.
    DemandGeneratorFunc demand_generator_func_;
};