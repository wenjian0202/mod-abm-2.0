# `mod-abm-2.0` 
A modern agent-based modeling platform for mobility-on-demand simulations.

<img src="https://github.com/wenjian0202/mod-abm-2.0/blob/main/doc/hongkong.gif" width="700">

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

The simulation `Platform` runs in conjunction with a `Router` and a `DemandGenerator`. During the process, in each cycle, `Platform` invokes `DemandGenerator` to generate trip requests based on the pre-defined ODs and trip intensities. It then dispatches the trips to vehicles following the selected dispatching strategies. `Router` , which wraps around the `OSRM` backend, provides the shortest routes for trips during dispatching.

<img src="https://github.com/wenjian0202/mod-abm-2.0/blob/main/doc/diagram.svg" width="700">

The program outputs simulation results in two formats:
- a report as a quick summary of the simulation results (through terminal)
- a detailed datalog for debugging, visualization and in-depth investigation (in `.yml`)

We also provide Python tools to parse the output datalog and create animation in `.mp4` format. Example of the rendered video can be found at [demo.mp4](https://github.com/wenjian0202/mod-abm-2.0/tree/main/media/demo.mp4).

As of today, `mod-abm-2.0` has the following developped or planned features:
- Dispatching
  - Insertion Heuristics, which assigns trips to vehicles while minimizes the total travel time of all travelers
  - [WIP, High Priority] interface with machine learning libraries for ML-based dispatching
  - [TBD, Mid Priority] re-optimization through meta-heuristics for better trip-vehicle matching
  - [TBD, low Priority] rebalancing of idle vehicles for better levels of service
- Router
  - `OSRM` routing engine on static map (that stores locally)
  - [TBD, Low Priority] routing with dynamic traffic data or historic traffic data
- Demand Generator
  - time-invariant demand (that generates following Poisson distribution)
  - [TBD, High Priority] mode choice model that competes with transit and other mode of transportation
- Others
  - [TBD, Mid Priority] simulated transit systems for multi-modal simulations 

Full documentation of `mod-abm-2.0` can be found below. 

## Ducumentation

- [Quick Start](https://github.com/wenjian0202/mod-abm-2.0/blob/main/doc/quick-start.md) to get started with `mod-abm-2.0`.
- [Runbook](https://github.com/wenjian0202/mod-abm-2.0/blob/main/doc/runbook.md) to understand how everything works under the hood and how to set up your own scenarios.
- [FAQ](https://github.com/wenjian0202/mod-abm-2.0/blob/main/doc/faq.md) for frequently asked questions.

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

