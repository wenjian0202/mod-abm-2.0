# `mod-abm-2.0`
An agent-based modeling platform for mobility-on-demand simulations. **WORK IN PROGRESS**.

<img src="https://github.com/wenjian0202/mod-abm-2.0/blob/main/media/demo.gif" width="400">

## What's `mod-abm-2.0`?

`mod-abm-2.0` is the 2.0 version of our **a**gent-**b**ased **m**odeling platform designed to simulte the operations of **m**obility-**o**n-**d**emand systems. The software largely extends and rewrites the original [`amod-abm`](https://github.com/wenjian0202/amod-abm/), which was first developed in 2016 as part of an MIT research project. A lot of new designs have been made in this 2.0 version in pursuit of better scalability and extensibility, including:
- C++ with native [OSRM](https://github.com/Project-OSRM/osrm-backend) instead of Python to allow for much optimized runtime performance;
- customization through user-defined system configuration and swappable modules for demand/dispatching to support different use cases and research interests;
- thorough tests, good documentation and modern software development practices.

`mod-abm-2.0` is able to simulate mobility-on-demand systems with *agents* (travelers, vehicles, couriers etc.) at the individual level. It provides infrastructure and tools to
- support design and operational decisions (such as fleet size, pricing policies) for MoD systems, shared or non-shared, autonomous or non-autonomous;
- model traveler behavior and traffic demand;
- research smart dispatch algorithms w.r.t trip-vehicle assignment, fleet management, and rebalancing, including machine-learning models.
Almost effortlessly, this software could be transfered to any urban setting for fast simulation of city-level operations. The larged decoupled system also lends itself to customized extensions according to developers' own needs. You're all welcome to contribute to `mod-abm-2.0`! 

## what is included?

In construction.

## Quick Start

In construction.

## Support

You can post bug reports and feature requests in [Issues](https://github.com/wenjian0202/mod-abm-2.0/issues).

## Relavant Papers

- Wen, J., Chen, Y.X., Nassir, N. and Zhao, J., 2018. Transit-oriented autonomous vehicle operation with integrated demand-supply interaction. *Transportation Research Part C: Emerging Technologies*, 97, pp.216-234. ([pdf](https://www.researchgate.net/profile/Jinhua_Zhao8/publication/323791652_Rebalancing_shared_mobility-on-demand_systems_A_reinforcement_learning_approach/links/5d46d8ca299bf1995b645644/Rebalancing-shared-mobility-on-demand-systems-A-reinforcement-learning-approach.pdf))

- Wen, J., Zhao, J. and Jaillet, P., 2017, October. Rebalancing shared mobility-on-demand systems: A reinforcement learning approach. *2017 IEEE 20th International Conference on Intelligent Transportation Systems (ITSC)* (pp. 220-225). Ieee. ([pdf](https://www.researchgate.net/profile/Jinhua_Zhao8/publication/323791652_Rebalancing_shared_mobility-on-demand_systems_A_reinforcement_learning_approach/links/5d46d8ca299bf1995b645644/Rebalancing-shared-mobility-on-demand-systems-A-reinforcement-learning-approach.pdf))

- Wen, J., Nassir, N. and Zhao, J., 2019. Value of demand information in autonomous mobility-on-demand systems. *Transportation Research Part A: Policy and Practice*, 121, pp.346-359. ([pdf](https://dusp.mit.edu/sites/dusp.mit.edu/files/attachments/publications/Value_of_Information.pdf))

## Citing

If you use `mod-abm-2.0` in your research, you can cite it as follows:

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

