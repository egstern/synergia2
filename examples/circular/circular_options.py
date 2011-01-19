#!/usr/bin/env python

import synergia_workflow

opts = synergia_workflow.Options("circular")
opts.add("num_macro_particles", 1000, "Number of macro particles", int)
opts.add("seed", 0, "Pseudorandom number generator seed", int)
opts.add("num_real_particles", 1.1e11, "Number of real particles", float)
opts.add("verbose", True, "Verbose propagation", bool)
opts.add("num_steps", 8, "Number of steps per turn", int)
opts.add("num_turns", 20, "Number of turns", int)
opts.add("map_order", 1, "Map order", int)
opts.add("norm_emit",5.89533703303356e-07, "Horizontal and vertical emittance [m rad]", float)
opts.add("stdz", 0.05, "RMS longitudinal length [m]", float)
opts.add("harmno",32,"Harmonic number", int)
opts.add("rf_voltage", 1.0e-3, "RF cavity voltage in MV", float)
opts.add("x_offset", 0.0, "Bunch offset in x", float)
opts.add("y_offset", 0.0, "Bunch offset in y", float)
opts.add("z_offset", 0.0, "Bunch offset in z", float)

job_mgr = synergia_workflow.Job_manager("circular.py", opts, ["foborodobo_s.lat"])
