## Quick Start

The following guideline has been tested and verified on MacOS. Linux users should expect very similar approaches.

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

[`OSRM`](https://github.com/Project-OSRM/osrm-backend) is a high-performance routing engine that runs on [OpenStreetMap](https://www.openstreetmap.org/) data. We use `OSRM` backend libraries in `mod-abm-2.0` for its brilliant runtime performance.

We recommend that `OSRM` and its other dependencies are installed in a separate folder, structured as below for maintainability:
```
|-- mod-abm-2.0
|-- osrm
   |-- map
   |-- lua-5.3.6
   |-- osrm-backend
```
Start from the desired working directory, we create an `osrm` folder and jump into it.
```
mkdir osrm
cd osrm
```
Manually install `lua@5.3` by running the following commands:
```
curl -R -O http://www.lua.org/ftp/lua-5.3.6.tar.gz
tar zxf lua-5.3.6.tar.gz
cd lua-5.3.6
make macosx test
make install
cd ..
```
Clone `osrm-backend` to local `osrm` folder and install it.
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

We will also need a map that the routing engine (hence our simulation platform) operates on. The raw [OpenStreetMap](https://www.openstreetmap.org/) map data are encoded in `*.osm.pbf` format and many map extract [servers](https://wiki.openstreetmap.org/wiki/Planet.osm) offer downloading the whole world map or that of a specific region/country. [Protomaps](https://protomaps.com/extracts) is one of the best that allows us to extract rectangular area or draw polygon. This is ideal for small case study areas or city-scale simulations. For larger map areas (say, California of US, or Germany as a whole), [Geofabrik](https://download.geofabrik.de/) is a good alternative that provides country/state-level extracts that are updated daily.
 
In our case, Hong Kong will be the area of interest (thanks to our HKU partnership!). Make a `map` subfolder: 
```
mkdir map
```
We download the Hong Kong map extract from [Protomaps](https://protomaps.com/extracts) and name it `hongkong.osm.pbf`. Make sure we have moved the downloaded `*.osm.pbf` file into the `osrm/map` subfolder before continuing.

`OSRM` requires several steps to pre-process the map data, which largely enhances the routing performance by allowing the use of Multi-Level Dijkstra (MLD). To make that happen, run:
```
./osrm-backend/build/osrm-extract ./map/hongkong.osm.pbf -p osrm-backend/profiles/car.lua
./osrm-backend/build/osrm-partition ./map/hongkong.osrm
./osrm-backend/build/osrm-customize ./map/hongkong.osrm
```

### Build `mod-abm-2.0`

Okay, we can now go back to the original working directory and clone the `mod-abm-2.0` repo.
```
cd ..
git clone https://github.com/wenjian0202/mod-abm-2.0.git
cd mod-abm-2.0
```
To configure the build system, run once:
```
cmake -S . -B build
```
Future builds can be run as simply as:
```
cmake --build build
```

Once the build is complete, try the exmaple command line that runs the demo simulation:
```
./build/main "./config/platform_demo.yml" "../osrm/map/hongkong.osrm" "./config/demand_demo.yml" 1
```
It runs smoothly and outputs a report? Hooray! Now we've set up the `mod-abm-2.0` simulation platform. To understand how everything works under the hood and how to set up your own scenarios, please follow our [Runbook](https://github.com/wenjian0202/mod-abm-2.0/blob/main/doc/RUNBOOK.md).

Questions？Please check out our [FAQ](https://github.com/wenjian0202/mod-abm-2.0/blob/main/doc/FAQ.md). You can also post bug reports and feature requests in [Issues](https://github.com/wenjian0202/mod-abm-2.0/issues).
