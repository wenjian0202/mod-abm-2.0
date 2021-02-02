/// \author Jian Wen
/// \date 2021/01/29

#include "config.hpp"
#include "demand_generator.hpp"
#include "router.hpp"
#include "types.hpp"

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>

int main(int argc, const char *argv[])
{
    // Check the input arugment list
    if (argc != 3)
    {
        fmt::print(stderr, "[ERROR] We need exact 2 arguments aside from the program name for correct execution! \n"
                           "- Usage: <prog name> <arg1> <arg2>. The <arg1> is the path to the platform config file. The <arg2> is the path to the demand config file.\n"
                           "- Example: {} \"./config/platform.yml\" \"./config/demand.yml\"\n",
                   argv[0]);
        return -1;
    }

    // Load the platform config from the external yaml file and convert it to c++ struct
    auto platform_config = load_platform_config(argv[1]);

    // Initiate the router with the osrm data
    Router router{platform_config.path_to_osrm_data};

    // Load the demand for the demand generator
    DemandGenerator demand_generator{argv[2], platform_config.simulation_config.cycle_s};

    demand_generator();
    demand_generator();
    demand_generator();
    demand_generator();
    demand_generator();
    demand_generator();
    demand_generator();
    demand_generator();
    demand_generator();
    demand_generator();

    // Pos origin{113.93593149478123, 22.312648328005512};
    // Pos destination{114.13602296340699, 22.28328541732128};

    // auto ret = router(origin, destination);

    return 0;
}
