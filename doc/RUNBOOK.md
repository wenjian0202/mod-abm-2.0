## Runbook

### Demo Walk-through

The [`main()`](https://github.com/wenjian0202/mod-abm-2.0/blob/main/src/main.cpp) function is the gateway to the simulation. To invoke `main()` and run the demo simulation scenario, do:
```
./build/main "./config/platform_demo.yml" "../osrm/map/hongkong.osrm" "./config/demand_demo.yml" 1
``` 

This function takes three compulsory external files as input:
- a platform config file (in `.yml`  format)
- a pre-processed map data file for [`OSRM`](https://github.com/Project-OSRM/osrm-backend) routing engine (in `.osrm` format, not to confuse with the raw `.osm.pbf` data extract directly downloaded from OSM servers)
- a demand config file (in `.yml` format)

You can customize the each of the input config files to, for example, switch to a different area, modify fleet size and vehicle capacity, or use a different demand matrix. 

One more optional argument, in addition to the three compulsory ones, is the seed (an unsigned integer) for the random number generator. Using the same seed in multiple runs allows for reproducing the same sim results deterministically. If not provided, the simulation will use the current time as seed (and you end up having fresh results in each new run).

The simulation runs and outputs a report that summarizes the system performance such as runtime and average travel time. An example report is found below:
```
----------------------------------------------------------------------------------------------------------------
# System Configurations
 - Simulation Config: simulation_duration = 3000s (main simulation between 1200s and 1800s).
 - Fleet Config: fleet_size = 8, vehicle_capacity = 2.
 - Request Config: max_wait_time = 900s.
 - Output Config: output_datalog = true, render_video = true.
# Simulation Runtime
 - Runtime: total_runtime = 20.3413s, average_runtime_per_simulated_second = 0.00678044.
# Trips
 - Total Trips: requested = 12 (of which 10 dispatched [83.3333%] + 2 walked away [16.6667%]).
 - Travel Time: completed = 5. average_wait_time = 521.36s, average_travel_time = 477.921s.
# Vehicles
 - Distance: average_distance_traveled = 7176.35m. average_distance_traveled_per_hour = 43058.1m.
 - Load: average_load = 1.07387.
----------------------------------------------------------------------------------------------------------------
```

You may have already notices that, the platform config has two flags, `output_datalog` and `render_video`, which determine if we write additional data to a datalog for future analysis and video rendering. In this demo, both flags are on, and the previous simulation run should have created an output datalog at `datalog/demo.yml`.  

To create animation video for the simulation run, we actually provide Python tools that do it in a two-step process. First, run once to fetch the background map image for a given area from OSM server:
```
python3 ./python/fetch_map.py "./config/platform_demo.yml" "./media/hongkong.png"
```
The fetched image will be saved as `hongkong.png` under `/media` folder. Unless you change your area of interest (defined by the max/min longitudes and latitudes in the platform config), there is no need to rerun the fetch. We can then create animation for the previous simulation run by calling:
```
python3 ./python/render_video.py "./config/platform_demo.yml" "./media/hongkong.png"
```
This script will parse the datalog the platform config points to, and animate the vehicles and trips in the simulated MoD system onto the map image. This process takes a while, but with a little patience, you will now have a nice beautiful video clip saved to `/media/demo.mp4`. Play and watch!

### Understand the `Platform`

The class `Platform` does the heavy-lifting work inside `main()`. It initializes with a platform config, and interacts with two injected functors, `Router` and `DemandGenerator`. 

The platfrom config includes parameters for the following subsystems:
- `area_config` that defines the area boundary (the max/min longitudes and latitudes);
- `mod_system_config` that describes the fleet and trip requests;
- `simulation_config` such as simulation duration and cycle time;
- `output_config` that controls the output of datalog and the rendering of videos.

Note that the entire simulation duration consists of three stages: warm-up, main simulation, and wind-down. In warm-up, the simulation starts with the initial setup and empty vehicles and builds states as time evolves and trips come in. The main simulation follows, which is the actual "period of study" when the trips are counted in data analysis and result reports. The video rendering also applies only to the main simulation stage. The winddown is the closing stage when new trips are still generated but not counted. During the winddown, trips generated in the main stage will be completed (aka dropped off) to be able to have a fair understanding about their travel times.   

In each cycle during the simulation, `Platform` invokes `DemandGenerator` to generate trip requests based on the demand model. The current demand model takes in a list of trip OD pairs with intensities. It then generates trips whose interarrival times follow the Poission process. `Platform` then dispatches the trips to vehicles by inserting each of the trips to the vehicle that minimizes the total expected travel time. If the vehicle's capacity is more than one, trips can be shared with arbitrary pickup and dropoff orders (each pickup/dropoff is a "waypoint"). 

After dispatching, each individual vehicle will navigate itself through the planned waypoints to serve the trips sequenctially. If in the main simulation stage, `Platform` will accumuate the states along the process, such as distance traveled, load of vehicle, actual wait time and travel time etc. The states will be used to study the simulated MoD system from perspectives like fleet usage, level of service etc.

### Set Up Your Own Scenario

To set up your own scenario of MoD simulation, here is a list of all that are required:
- Select your area of interest, defined as the max/min longitudes and latitudes, and download the `*.osm.pbf` extract from a server such as [Protomaps](https://protomaps.com/extracts) or [Geofabrik](https://download.geofabrik.de/) that covers the entire area.
- Pre-process your map extract. You will run three command lines, `osrm-extract`, `osrm-partition` and `osrm-customize`, sequentially and the end result is a `*.osrm` file (see [Quick Start](https://github.com/wenjian0202/mod-abm-2.0/blob/main/doc/QUICKSTART.md)).
- Customize your platform config with your own fleet definition, request patterns etc (and make sure the max/min lon/lat are consistent!).
- Create your own demand config with a list of expected trip origins and destination, and their intensities.

...and that is it! Run `main()` with your own configs/map to see how this is like. Have fun! 

Questions？Please check out our [FAQ](https://github.com/wenjian0202/mod-abm-2.0/blob/main/doc/FAQ.md). You can also post bug reports and feature requests in [Issues](https://github.com/wenjian0202/mod-abm-2.0/issues).
