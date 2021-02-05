import matplotlib.pyplot as plt
import yaml
import smopy

# Describe the boundary of the simulated area.
lon_min = 113.90
lon_max = 114.25
lat_min = 22.10
lat_max = 22.60

# Load the boundary of the simulated area from config file.
# with open('./config/platform.yml') as file:
#     config = yaml.load(file, Loader=yaml.FullLoader)

#     lon_min = config["area_config"]["lon_min"]
#     lon_max = config["area_config"]["lon_max"]
#     lat_min = config["area_config"]["lat_min"]
#     lat_max = config["area_config"]["lat_max"]


map = smopy.Map((lat_min, lon_min, lat_max, lon_max), z=10) # ((lon_max, lon_min, lat_max, lat_min), z=10)
map.show_mpl()

plt.show()

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