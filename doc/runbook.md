## Runbook

### Demo Walk-through

The [`main()`](https://github.com/wenjian0202/mod-abm-2.0/blob/main/src/main.cpp) function is the gateway to the simulation. To invoke `main()` and run the demo simulation scenario, do:
```
./build/main "./config/platform_demo.yml" "../osrm/map/hongkong.osrm" "./config/demand_demo.yml" 1
``` 

This function takes three compulsory external files as input:
- platform config file (in `.yml`  format)
- pre-processed map data file for [`OSRM`](https://github.com/Project-OSRM/osrm-backend) routing engine (in `.osrm` format, not to confuse with the raw `.osm.pbf` file)
- demand config file (in `.yml` format)
You can customize the input config files to, for example, switch to a different area, modify fleet size and vehicle capacity, or use a different demand matrix. One more optional argument, in addition to the three arguments, is the seed (an unsigned integer) for the random number generator. Using the same seed in multiple runs allows for reproducing the same sim results deterministically. If not provided, the simulation will use the current time as seed (and you end up having fresh results in each new run).

The simulation runs and outputs a report that summerizes the system performance such as runtime and average travel time. An example report is found below:
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

You may have already notices that, the platform config has two flags, `output_datalog` and `render_video`, which determines the needs to write additional data to a datalog for future analysis and video rendering. In this demo, both flags are on, and the simulation run should have created an output datalog at `datalog/demo.yml`.  

To create animation video for the simulation run, we actually provide Python tools that do it in a two-step process. First, run once to fetch the background map image for a given area:
```
python3 ./python/fetch_map.py "./config/platform_demo.yml" "./media/hongkong.png"
```
The fetched image will be saved as `hongkong.png` under `/media` folder. We can then create animation for the previous simulation run by calling:
```
python3 ./python/render_video.py "./config/platform_demo.yml" "./media/hongkong.png"
```
You will now have a nice beautiful video clip saved to `/media/demo.mp4`. Check it out!

### Understand the `Platform`

The class `Platform` does the heavy-lifting work inside `main()`. 

`Platform` initializes with the platform config, and interacts with two injected functors, `Router` and `DemandGenerator`. 

During the simulation, in each cycle, `Platform` invokes `DemandGenerator` which generates trip requests based on the pre-defined ODs and trip intensities in the demand config. `Platform` then dispatches the trips to vehicles using the selected dispatching strategies (currently, inserting trips to vehicles that minimize the total wait time). When dispatching, `Router` , which wraps around the `OSRM` backend, is called at high frequency to provide the shortest routes on demand for any origin/destination pair.

TBD.

### Set Up Your Own Scenario

TBD.