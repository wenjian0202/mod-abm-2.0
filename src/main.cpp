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
    // Check the input arugment list.
    if (argc != 4)
    {
        fmt::print(stderr, "[ERROR] We need exact 3 arguments aside from the program name for correct execution! \n"
                           "- Usage: <prog name> <arg1> <arg2> <arg3> <arg4>. \n"
                           "  <arg1> is the path to the platform config file. \n"
                           "  <arg2> is the path to the orsm map data. \n"
                           "  <arg3> is the path to the demand config file. \n"
                           "- Example: {} \"./config/platform.yml\" \"../osrm/data/china-latest.osrm\" \"./config/demand.yml\" \n",
                   argv[0]);
        return -1;
    }

    // Initiate the router with the osrm data
    Router router{argv[2]};

    // Create the demand generator based on the input demand file.
    DemandGenerator demand_generator{argv[3]};

    // Create the simulation platform with the config loaded from file.
    auto platform_config = load_platform_config(argv[1]);

    Platform<decltype(router), decltype(demand_generator)> platform{
        std::move(platform_config),
        std::move(router),
        std::move(demand_generator)};

    // Run simulation.
    platform.run_simulation();

    return 0;
}
