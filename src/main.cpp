/// \author Jian Wen
/// \date 2021/01/29

#include "router.hpp"
#include "types.hpp"

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>

int main(int argc, const char *argv[])
{
    // Check the input arugment list
    if (argc != 2)
    {
        fmt::print(stderr, "Error! We need exact 1 argument aside from the program name for correct execution! \n"
                           "- Usage: <prog name> <arg1>. The <arg1> is the path to the platform config file.\n"
                           "- Example: {} \"/Users/jihan/Documents/mod-abm-2.0/config/platform.yml\" \n",
                   argv[0]);
        return -1;
    }

    // Load the platform config from the external yaml file and convert it to c++ struct
    auto platform_config_yaml = YAML::LoadFile(argv[1]);
    auto platform_config = convert_yaml_to_platform_config(platform_config_yaml);

    // Initiate the router witht the osrm data
    Router router(platform_config.path_to_osrm_data);

    Pos origin{113.93593149478123, 22.312648328005512};
    Pos destination{114.13602296340699, 22.28328541732128};

    auto ret = router(origin, destination);

    return 0;
}
