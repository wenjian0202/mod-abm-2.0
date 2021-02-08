import numpy as np
import matplotlib.pyplot as plt

from PIL import Image

import sys
import yaml


def get_boundary_from_config(path_to_config_file):
    """ Load the boundary of the simulated area from config file. """
    with open("./config/platform.yml") as file:
        config = yaml.load(file, Loader=yaml.FullLoader)

        lon_min = config["area_config"]["lon_min"]
        lon_max = config["area_config"]["lon_max"]
        lat_min = config["area_config"]["lat_min"]
        lat_max = config["area_config"]["lat_max"]

        return (lon_min, lon_max, lat_min, lat_max)


# map = smopy.Map((lat_min, lon_min, lat_max, lon_max), z=11, margin=None) # ((lon_max, lon_min, lat_max, lat_min), z=10)
# map.show_mpl()

# plt.show()

# with open('datalog/datalog.yml') as file:
#     datalog = yaml.load(file, Loader=yaml.FullLoader)

#     vehicles = datalog[1]["vehicles"]
#     waypoints = vehicles[0]["waypoints"]

#     lons = []
#     lats = []
#     for step in waypoints[0]:
#         lons.append((step["lon"] - lon_min)/(lon_max - lon_min))
#         lats.append((step["lat"] - lat_min)/(lat_max - lat_min))

#     plt.plot(lons, lats)

#     plt.show()

def main():
    # Check command line arguments.
    if (len(sys.argv) != 3):
        print("[ERROR] We need exact 2 arguments aside from the program name for correct execution! \n"
                           "- Usage: python3 <prog name> <arg1> <arg2>. \n"
                           "  <arg1> is the path to the platform config file. \n"
                           "  <arg2> is the path to the background map image. \n"
                           "- Example: python3 {} \"./config/platform.yml\" \"./media/hongkong.png\"\n".format(sys.argv[0]))

        sys.exit(1)

    print("Loading the boundary of the simulated area from config at {}.".format(sys.argv[1]))

    lon_min, lon_max, lat_min, lat_max = get_boundary_from_config(sys.argv[1])

    print("Loaded boundary: lon_min={}, lon_max={}, lat_min={}, lat_max={}.".format(lon_min, lon_max, lat_min, lat_max))

    img = Image.open(sys.argv[2])
    w, h = img.size

    with open('datalog/datalog.yml') as file:
        datalog = yaml.load(file, Loader=yaml.FullLoader)

        vehicles = datalog[1]["vehicles"]
        waypoints = vehicles[0]["waypoints"]

        lons = []
        lats = []
        for step in waypoints[0]:
            lons.append((step["lon"] - lon_min) / (lon_max - lon_min) * w)
            lats.append((lat_max - step["lat"]) / (lat_max - lat_min) * h)

        fig, ax = plt.subplots()
        ax.imshow(img)

        ax.plot(lons, lats)

        # ax.spines['top'].set_visible(False)
        # ax.spines['left'].set_visible(False)
        # ax.spines['bottom'].set_visible(False)
        # ax.spines['right'].set_visible(False)
        # ax.set_xticks([])
        # ax.set_yticks([])
        plt.show()

if __name__ == "__main__":
    main()