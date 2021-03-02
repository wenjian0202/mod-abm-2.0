## Frequently Asked Questions

Q: How the simulation can be even faster?

A: First, for large scale simulations, please make sure to turn off both `output_datalog` and `render_video` flags in the platform config. Writing data (expecially those detailed ones for video rendering) to datalog can be very, very, very expensive. A couple of other options for speeding up simulation are:
- use a larger cycle (a cycle defines the time interval between the periodic dispatching events).
- use a smaller radius when searching for available vehicles to dispatch (TBD).

Just keep in mind, when trading for speed, we are giving up a little bit of our accuracy and optimality. 

Q: How can I suppress the verbose debugs?

`grep` is a helpful tool that searches for lines containing a match to the given strings or words. You can add `| grep [OPTIONS] PATTERN` to the simulation run to filter its output through piping. For example:
- `grep "Vehicle #2"` will only keep the lines that contain string `Vehicle #2`.
- `grep -v [DEBUG]` will filter out all `DEBUG` lines.
