/// \author Jian Wen
/// \date 2021/01/29

#include "config.hpp"
#include "demand_generator.hpp"
#include "platform.hpp"
#include "router.hpp"
#include "types.hpp"

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>

int main(int argc, const char *argv[])
{
    // Check the input arugment list
    if (argc != 4)
    {
        fmt::print(stderr, "[ERROR] We need exact 3 arguments aside from the program name for correct execution! \n"
                           "- Usage: <prog name> <arg1> <arg2> <arg3>. "
                           "The <arg1> is the path to the platform config file. "
                           "The <arg2> is the path to the orsm map data. "
                           "The <arg3> is the path to the demand config file.\n"
                           "- Example: {} \"./config/platform.yml\" \"../osrm/data/china-latest.osrm\" \"./config/demand.yml\"\n",
                   argv[0]);
        return -1;
    }

    // Load the platform config from the external yaml file and convert it to c++ struct
    auto platform_config = load_platform_config(argv[1]);

    // Initiate the router with the osrm data
    Router router{argv[2]};

    // Create the demand generator based on the input demand file
    DemandGenerator demand_generator{argv[3], platform_config.simulation_config.cycle_s};

    // Create the simulation platform
    Platform<decltype(router), decltype(demand_generator)> platform{std::move(platform_config), std::move(router), std::move(demand_generator)};

    platform.invoke_demand_generator();
    platform.invoke_demand_generator();
    platform.invoke_demand_generator();
    platform.invoke_demand_generator();
    platform.invoke_demand_generator();

    platform.invoke_router();

    return 0;
}
