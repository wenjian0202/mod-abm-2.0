# `mod-abm-2.0` 
A modern agent-based modeling platform for mobility-on-demand simulations.

<img src="https://github.com/wenjian0202/mod-abm-2.0/blob/main/media/demo.gif" width="700">

## What's `mod-abm-2.0`?

The `mod-abm-2.0` is the 2.0 version of our **A**gent-**B**ased **M**odeling platform designed for the simulation of large-scale **M**obility-**o**n-**D**emand operations. The software largely extends and rewrites its original version ([`amod-abm`](https://github.com/wenjian0202/amod-abm/)), which was first developed in 2016 as part of an MIT research project. In this 2.0 version, a lot of new designs have been made possible in pursuit of better scalability and extensibility. This includes:
- implementation in C++ with native [`OSRM`](https://github.com/Project-OSRM/osrm-backend) support instead of Python to allow for much optimized runtime performance;
- user-customizable simulations through hierarchical system configurations, decoupled routing/demand-generating modules and easily transferrable maps to support different use cases and research topics;
- thorough unit tests, conprehensive documentation and modern software development practices.

Almost effortlessly, `mod-abm-2.0` can be set up to simulate city-level mobility-on-demand systems in any urban settings. It creates a free-floating MoD system with a fleet of vehicles and a central dispatcher, and models each of the *agents* (travelers, vehicles, couriers etc.) at the individual level. The simulation will evaluate the system performance based on user-defined indicators, such as wait time, travel time, service rate at the traveler side, and vehicle miles traveled, average load, vehicle idle time at the operator side. Additional Python tools are also provided to create animation from the simulated data for debugging/visualization purposes. 

With all great features aforementioned, `mod-abm-2.0` creates opportunities for researchers and planners to:
- support design and operational decisions (such as fleet size, pricing policies) for MoD systems, shared or non-shared, autonomous or non-autonomous;
- model traffic demand and understand travel behavior;
- evaluate smart dispatch algorithms w.r.t trip-vehicle assignment, fleet management, and rebalancing, including machine-learning models.

Thanks for contributing to `mod-abm-2.0`! 

## `mod-abm-2.0` Explained

The [`main()`](https://github.com/wenjian0202/mod-abm-2.0/blob/main/src/main.cpp) function of the simulation takes three external files as input:
- platform config file (in `.yml`)
- pre-processed map data file for [`OSRM`](https://github.com/Project-OSRM/osrm-backend) routing engine (in `.osrm`)
- demand config file (in `.yml`)

The meat part of the simulation is managed by class `Platform`, which also interacts with two injected functors `Router` and `DemandGenerator`. During the simulation, in each cycle, `Platform` invokes `DemandGenerator` to generate trip requests that follows the Poisson process, based on the pre-defined ODs and trip intensities. It then dispatches the trips to vehicles using the selected dispatching strategies (currently, minimize total wait time). When dispatching, `Router` , which wraps around the `OSRM` backend, is called at high frequency to provide the shortest routes on demand for any origin/destination pair.

<img src="https://github.com/wenjian0202/mod-abm-2.0/blob/main/media/diagram.svg" width="700">

The program outputs simulation results in two formats:
- a report as a quick summary of the simulation results (through terminal)
- a detailed datalog for debugging, visualization and in-depth investigation (in `.yml`)

We also provide Python tools to parse the output datalog and create animation through a two-step process. First, `fetch_map.py` fetches the background map image for the visualization from [OpenStreetMap](https://www.openstreetmap.org/) (you only do it once unless you change the area of study). `render_video.py` then creates animation on top of the background map image using the captured simulation states at each frame from the datalog, and renders video as `.mp4`.

Example of the input configs, output datalog, as well as the rendered video could be found in the relavant folders ([config](https://github.com/wenjian0202/mod-abm-2.0/tree/main/config), [datalog](https://github.com/wenjian0202/mod-abm-2.0/tree/main/datalog), [media](https://github.com/wenjian0202/mod-abm-2.0/tree/main/media)) of this repo. 

### Ongoing Parts

- performance improvement through cached routing results and compressed route representation
- more debugging printouts, more documentation

### Upcoming Features 

- re-optimization of the vehicle-trip assignment for better MoD performance
- time-variant demand matrix

## Quick Start

The following guideline applies to MacOS. Linux users should expect very similar approaches.

### Install Dependencies

To start, install [Homebrew](https://brew.sh/), a software package manager that helps install all other dependencies.
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
Install `wget` (and verify `brew` is working properly):
```
brew install wget
```
And install all other dependencies:
```
brew install git 
brew install cmake python ffmpeg
brew install boost libzip libxml2 tbb ccache GDAL
```

:warning:  As of today, `OSRM` ([`v5.24.0`](https://github.com/Project-OSRM/osrm-backend/releases/tag/v5.24.0)) does not work well with the latest `lua` ([`lua@5.4`](https://www.lua.org/versions.html#5.4)) release. We will manually install `lua@5.3` later until `OSRM` supports the new `lua` version.

### Setup OSRM

[`OSRM`](https://github.com/Project-OSRM/osrm-backend) is a high-performance routing engine that runs on [OpenStreetMap](https://www.openstreetmap.org/) data. We  leverage `OSRM` backend libraries in `mod-abm-2.0` for better runtime performance.

We recommend that `OSRM` and its other dependencies are installed in a separate folder, structured as below for maintainability:
```
|-- mod-abm-2.0
|-- osrm
   |-- map
   |-- lua-5.3.6
   |-- osrm-backend
```
Start from the desired working directory, we create a `osrm` folder and jump into it.
```
mkdir osrm
cd osrm
```
Manually install `lua@5.3`.
```
curl -R -O http://www.lua.org/ftp/lua-5.3.6.tar.gz
tar zxf lua-5.3.6.tar.gz
cd lua-5.3.6
make macosx test
make install
cd ..
```
Clone `osrm-backend` and install it.
```
git clone https://github.com/Project-OSRM/osrm-backend.git
cd osrm-backend
mkdir build
cd build
cmake ..
make
make install
cd ..
cd ..
```
We also need a map that the routing engine (hence our simulation platform) will operate on. The raw [OpenStreetMap](https://www.openstreetmap.org/) map data are encoded in `*.osm.pbf` format and many map extract [servers](https://wiki.openstreetmap.org/wiki/Planet.osm) offers downloading the whole world map or that of a specific region/country. [Protomaps](https://protomaps.com/extracts) is one of the best that allow us to extract rectangular area or draw polygon (good for small case study of city-scale operations). For larger areas (say, California of US, or Germany as a whole), [Geofabrik](https://download.geofabrik.de/) is another good alternative.
 
In our case, Hong Kong will be the area of interest (thanks to our HKU partnership!). Make a `map` subfolder: 
```
mkdir map
```
We download the Hong Kong map extract from [Protomaps](https://protomaps.com/extracts) and name it `hongkong.osm.pbf`. Make sure we have moved the downloaded file into the `osrm/map` subfolder before the following steps.

Several more additional steps are required to pre-process the map data, which largely enhance the routing capabilities by allowing the use of Multi-Level Dijkstra (MLD).
```
./osrm-backend/build/osrm-extract ./map/hongkong.osm.pbf -p osrm-backend/profiles/car.lua
./osrm-backend/build/osrm-partition ./map/hongkong.osrm
./osrm-backend/build/osrm-customize ./map/hongkong.osrm
```

### Run `mod-abm-2.0`

Okay, we can now go back to the original working directory and clone the repo.
```
cd ..
git clone https://github.com/wenjian0202/mod-abm-2.0.git
cd mod-abm-2.0
```
To configure the build system, run:
```
cmake -S . -B build
```
Future builds can be run as simply as:
```
cmake --build build
```

An exmaple command line for running the simulation is:
```
./build/main "./config/platform.yml" "../osrm/map/hongkong.osrm" "./config/demand.yml"
```
Note that the `main()` function takes three arguments: platform config file, pre-processed map data file (in `.osrm` format, not to confuse with the raw `.osm.pbf` file) and demand config file. You can customize the config files to, for example, change fleet size, or use a different demand matrix.

The simulation runs and outputs a report. An example report is found below:
```
----------------------------------------------------------------------------------------------------------------
# System Configurations
 - Simulation Config: simulation_duration = 3000s (1200s warm-up + 600s main + 1200s wind-down).
 - Fleet Config: fleet_size = 8, vehicle_capacity = 2.
 - Request Config: max_wait_time = 900s.
 - Output Config: output_datalog = true, render_video = true.
# Simulation Runtime
 - Runtime: total_runtime = 35.6115s, average_runtime_per_simulated_second = 0.0118705.
# Trips
 - Total Trips: requested = 12 (of which 9 dispatched [75%] + 3 walked away [25%]).
 - Travel Time: completed = 6. average_wait_time = 777.545s, average_travel_time = 390.767s.
# Vehicles
 - Distance: average_distance_traveled = 7327.46m. average_distance_traveled_per_hour = 43964.8m.
 - Load: average_load = 1.08141.
----------------------------------------------------------------------------------------------------------------
```

Depending on the config, the program may also output the datalog to an `.yml` file. If you are to create animation, run once to fetch the background map image for the given area:
```
python3 ./python/fetch_map.py "./config/platform.yml" "./media/hongkong.png"
```
We can then create animation for each simulation run (as long as `output_datalog` and `render_video` in config are on) by calling:
```
python3 ./python/render_video.py "./config/platform.yml" "./media/hongkong.png"
```
You will now have a nice beautiful video clip saved to the desired path. Enjoy!

## Frequently Asked Questions

Q: How the simulation can be even faster?

A: First, for large scale simulations, please make sure to turn off `output_datalog` and `render_video` flags in the platform config. Writing data (expecially those detailed ones for video rendering) to log can be very very expensive. A couple of other options for speeding up simulation are:
- use a larger cycle (a cycle defines the interval between the periodic dispatching events)
- use a smaller radius when searching for available vehicles to dispatch (TBD)

Just keep in mind, when trading for speed, we are giving up a little bit of our accuracy and optimality. 

## Support

You can post bug reports and feature requests in [Issues](https://github.com/wenjian0202/mod-abm-2.0/issues).

## Relavant Papers

- Wen, J., Chen, Y.X., Nassir, N. and Zhao, J., 2018. Transit-oriented autonomous vehicle operation with integrated demand-supply interaction. *Transportation Research Part C: Emerging Technologies*, 97, pp.216-234. ([pdf](https://www.researchgate.net/profile/Jinhua_Zhao8/publication/323791652_Rebalancing_shared_mobility-on-demand_systems_A_reinforcement_learning_approach/links/5d46d8ca299bf1995b645644/Rebalancing-shared-mobility-on-demand-systems-A-reinforcement-learning-approach.pdf))

- Wen, J., Zhao, J. and Jaillet, P., 2017, October. Rebalancing shared mobility-on-demand systems: A reinforcement learning approach. *2017 IEEE 20th International Conference on Intelligent Transportation Systems (ITSC)* (pp. 220-225). Ieee. ([pdf](https://www.researchgate.net/profile/Jinhua_Zhao8/publication/323791652_Rebalancing_shared_mobility-on-demand_systems_A_reinforcement_learning_approach/links/5d46d8ca299bf1995b645644/Rebalancing-shared-mobility-on-demand-systems-A-reinforcement-learning-approach.pdf))

- Wen, J., Nassir, N. and Zhao, J., 2019. Value of demand information in autonomous mobility-on-demand systems. *Transportation Research Part A: Policy and Practice*, 121, pp.346-359. ([pdf](https://dusp.mit.edu/sites/dusp.mit.edu/files/attachments/publications/Value_of_Information.pdf))

## Citing

For research work/publications that uses `mod-abm-2.0`, please cite as follows:

```
@misc{wen2021-mod-abm-2.0,
    author = {Wen, Jian},
    title = {mod-abm-2.0, an agent-based modeling platform for mobility-on-demand simulations},
    year = {2021},
    publisher = {GitHub},
    journal = {GitHub repository},
    howpublished = {\url{https://https://github.com/wenjian0202/mod-abm-2.0}},
}
```

