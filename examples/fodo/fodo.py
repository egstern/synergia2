#!/usr/bin/env python

import numpy
import time
import math
import os
import sys

import synergia
import s2_fish
from diagnostics_file import Diagnostics_file

from mpi4py import MPI

from fodo_options import opts

def near_equal(a, b, tolerance=1e-6):
    retval = False
    if (a == 0):
        if (abs(b) < tolerance):
            retval = True
    else:
        if (abs(b / a - 1.0) < tolerance):
            retval = True
    return retval

def print_pass(a, b, tolerance=1e-6):
    if b == None:
        return "n/a"
    else:
        if near_equal(a, b, tolerance):
            return "pass"
        else:
            return "FAIL"
        
def summarize(diag, np):
    if MPI.COMM_WORLD.Get_rank() == 0:
        if (np == 0.0):
            expected = Diagnostics_file("fodo-np0.h5")
        elif near_equal(2.0e11, np):
            expected = Diagnostics_file("fodo-np2e11.h5")
        else:
            class Dummy():
                pass
            expected = Dummy()
            expected.s = [None]
            expected.mean = [None, None, None, None, None, None]
            expected.std = [[None], [None], [None], [None], [None], [None]]
        s = diag.get_s()
        last = len(s) - 1
        last_expected = len(expected.s) - 1
        print "%10s %15s %15s %20s %15s" % ("", "initial", "final", "expected", "pass/fail")
        print "%10s %15g %15g %20s %15s" % ("s", s[0], s[last], expected.s[last_expected],
                              print_pass(s[last], expected.s[last_expected]))
        names = ["x", "xp", "y", "yp", "t", "tp"]
        means = diag.get_means()
        for i in range(0, 6):
            if abs(expected.mean[last_expected, i])< 1.0e6:
                expected.mean[last_expected, i] = 0.0
            if (i == 4):
                print "%10s %15g %15g %20s %15s" % \
                    ("mean " + names[i], means[0, i], means[last, i],
                    expected.mean[last_expected, i],
                    "n/a")
            else:
                print "%10s %15g %15g %20s %15s" % \
                    ("mean " + names[i], means[0, i], means[last, i],
                    expected.mean[last_expected, i],
                    print_pass(expected.mean[last_expected, i], means[last, i]))
        stds = diag.get_stds()
        for i in range(0, 6):
    #        print "%10s %15g %15g" % ("std " + names[i], stds[0, i], stds[last, i])
            print "%10s %15g %15g %20s %15s" % \
                ("std " + names[i], stds[0, i], stds[last, i],
                expected.std[last_expected, i],
                print_pass(expected.std[last_expected, i], stds[last, i],5.0e-3))
        

if (__name__ == '__main__'):
    t0 = time.time()
    mass = synergia.PH_NORM_mp
    kinetic_energy = 1.5 - mass
    charge = 1.0
    initial_phase = 0.0
    scaling_frequency = 1.0e8
    part_per_cell = opts.get("partpercell")
    width_x = opts.get("xwidth")
    kicks_per_cell = opts.get("kickspercell")
    gridnum = opts.get("gridnum")
    griddim = (gridnum, gridnum, gridnum)
    num_particles = int(griddim[0] * griddim[1] * griddim[2] * part_per_cell)

    xoffset = opts.get("xoffset")  
    pipe_radius = opts.get("piperadius")
    pipe_conduct = opts.get("pipeconduct")
    space_charge = opts.get("spacecharge")
    solver = opts.get("solver")
    impedance = opts.get("impedance")
    bunchnp = opts.get("np")
    dpop = opts.get("dpop")

    ee = synergia.Error_eater()
    ee.start()
    gourmet = synergia.Gourmet(os.path.join(os.getcwd(), "fodo.lat"), "fodo", kinetic_energy,
                        scaling_frequency)
    gourmet.insert_space_charge_markers(kicks_per_cell) 
    
    
    beam_parameters = synergia.Beam_parameters(mass, charge, kinetic_energy,
                                         initial_phase, scaling_frequency,
                                         transverse=1)
    betagamma = beam_parameters.get_beta()*beam_parameters.get_gamma() 

    (alpha_x, alpha_y, beta_x, beta_y) = synergia.matching.get_alpha_beta(gourmet)
    emitx = (width_x) ** 2 / beta_x
    (width_xp, r_x, emit_x) = synergia.matching.match_twiss_width(width_x, alpha_x, beta_x)
    (width_y, width_yp, r_y) = synergia.matching.match_twiss_emittance(emit_x, alpha_y, beta_y)

    pz = beam_parameters.get_gamma() * beam_parameters.get_beta() * beam_parameters.mass_GeV
    beam_parameters.x_params(sigma=width_x, lam=width_xp * pz, r=r_x, offset=xoffset)
    beam_parameters.y_params(sigma=width_y, lam=width_yp * pz, r=r_y)
    sigma_z_meters = beam_parameters.get_beta()*synergia.PH_MKS_c / scaling_frequency / math.pi
    beam_parameters.z_params(sigma=sigma_z_meters, lam=dpop * pz)

    diag = synergia.Diagnostics(gourmet.get_initial_u())
    bunch = s2_fish.Macro_bunch.gaussian(bunchnp, num_particles, beam_parameters, diagnostics=diag, periodic=True)
    bunch.write_particles("begin")
    line_length = gourmet.orbit_length()
    tau = 0.5 * line_length / kicks_per_cell
    s = 0.0
    diag.add(s, bunch)
    
    if solver == "3D" or solver == "3d":
        solversp = "s2_fish_3d"
        sp_ch = s2_fish.SpaceCharge(solversp, grid=griddim, periodic=True)
#    elif solver == "3DC" or solver == "3dc":
#        pass
#    elif solver == "2D" or solver == "2d":
#         pass
    else:
        if MPI.COMM_WORLD.Get_rank() == 0:
            print "unknown solver '%s'" % solver
        sys.exit(1)

    repeats = 4
    for i in range(0, repeats):
        s = synergia.propagate(s, gourmet, bunch, space_charge=sp_ch)
        if MPI.COMM_WORLD.Get_rank() == 0:
            print "%d/%d complete" % (i+1,repeats)
    
    summarize(diag, bunchnp)
    
    if MPI.COMM_WORLD.Get_rank() == 0:
        print "elapsed time  =", time.time() - t0
    bunch.write_particles("end")
    diag.write_hdf5("fodo")
