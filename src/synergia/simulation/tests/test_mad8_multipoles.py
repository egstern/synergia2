#!/usr/bin/env python

import sys
sys.path.append('../../..')
import local_paths

from nose.tools import *
from synergia.foundation import Four_momentum, Reference_particle, pconstants
from synergia.lattice import Lattice, Mad8_reader, chef_beamline_as_string
from synergia.bunch import Bunch
from synergia.utils import Commxx, Logger
from synergia.simulation import Bunch_simulator, Independent_stepper, Propagator

import numpy as np

momentum = 1.5
multipole_sequence = "machine"
# read in a lattice, using the sequence name "machine"
# the reference particle for the lattice is a proton at momentum 1.5 GeV/c
def read_mad8_multipole_lattice(lattice_file):
    lattice = Mad8_reader().get_lattice(multipole_sequence, "./lattices/"+lattice_file+".lat")
    four_momentum = Four_momentum(pconstants.mp)
    four_momentum.set_momentum(momentum)
    refpart = Reference_particle(1, four_momentum)
    lattice.set_reference_particle(refpart)
    return lattice
    
# create the bunch of particles matching the test particles
# in the madx run
macro_particles = 32
real_particles = 1.0e10

def multipole_bunch(refpart):
    commxx = Commxx()
    bunch = Bunch(refpart, macro_particles, real_particles, commxx)
    s2o2 = np.sqrt(2.0)/2.0
    s3o2 = np.sqrt(3.0)/2.0
    lp = bunch.get_local_particles()

    # 16 particles at radius of 1.0e-3 at 30degrees and 45 degrees offsets
    # around axis
    offset = 1.0e-3
    lp[0, 0] = offset
    lp[0, 2] = 0.0

    lp[1, 0] = offset*s3o2
    lp[1, 2] = offset*0.5

    lp[2, 0] = offset*s2o2
    lp[2, 2] = offset*s2o2

    lp[3, 0] = offset*0.5
    lp[3, 2] = offset*s3o2

    lp[4, 0] = 0.0
    lp[4, 2] = offset

    lp[5, 0] = -offset*0.5
    lp[5, 2] =  offset*s3o2

    lp[6, 0] = -offset*s2o2
    lp[6, 2] =  offset*s2o2

    lp[7, 0] = -offset*s3o2
    lp[7, 2] =  offset*0.5

    lp[8, 0] = -offset
    lp[8, 2] = 0.0

    lp[9, 0] = -offset*s3o2
    lp[9, 2] = -offset*0.5

    lp[10, 0] = -offset*s2o2
    lp[10, 2] = -offset*s2o2

    lp[11, 0] = -offset*0.5
    lp[11, 2] = -offset*s3o2

    lp[12, 0] = 0.0
    lp[12, 2] = -offset

    lp[13, 0] =  offset*0.5
    lp[13, 2] = -offset*s3o2

    lp[14, 0] =  offset*s2o2
    lp[14, 2] = -offset*s2o2

    lp[15, 0] =  offset*s3o2
    lp[15, 2] = -offset*0.5

    # repeat same pattern shifted 0.5mm down 0.25mm
    x2_offset = 0.0005
    y2_offset = -0.00025
    for i in range(16,32):
        lp[i, 0] = lp[i-16, 0] + x2_offset
        lp[i, 2] = lp[i-16, 2] + y2_offset

    return bunch

def run_a_multipole(lattice_file_name):
    lattice = read_mad8_multipole_lattice(lattice_file_name)
    # print "lattice: ", lattice.as_string()
    bunch = multipole_bunch(lattice.get_reference_particle())
    bunch_simulator = Bunch_simulator(bunch)
    stepper = Independent_stepper(lattice, 1, 1)
    print("chef beamline: ", chef_beamline_as_string(stepper.get_lattice_simulator().get_chef_lattice().get_beamline()))
    propagator = Propagator(stepper)
    propagator.propagate(bunch_simulator, 1)
    m8p = np.load("./lattices/m8"+lattice_file_name+".npy")
    lp = bunch.get_local_particles()
    # for i in (0,4,8,12):
    #     print "lp[%d, :]: "%i, lp[i,:]
    #     print "m8p[%d, :]: "%i, m8p[i,:]
    #     print
    #numpart = lp.shape[0]
    numpart = bunch.get_local_num()
    assert(numpart == 32)
    for p in range(numpart):
        for j in [1]:
            print("    particle:  [%d, %d] %.14g <-> %.14g"%(p,j, lp[p,j], m8p[p,j]))
            assert_almost_equal(lp[p, j], m8p[p, j], 14)
    
def test_mpole_k1():
    run_a_multipole("mpole_k1")

def test_mpole_k1_tilt():
    run_a_multipole("mpole_k1_tilt")

def test_mpole_k2():
    run_a_multipole("mpole_k2")

def test_mpole_k2_tilt():
    run_a_multipole("mpole_k2_tilt")

def test_mpole_k3():
    run_a_multipole("mpole_k3")

def test_mpole_k3_tilt():
    run_a_multipole("mpole_k3_tilt")

def test_mpole_k4():
    run_a_multipole("mpole_k4")

def test_mpole_k4_tilt():
    run_a_multipole("mpole_k4_tilt")

def test_mpole_k5():
    run_a_multipole("mpole_k5")

def test_mpole_k5_tilt():
    run_a_multipole("mpole_k5_tilt")

def test_mpole_k6():
    run_a_multipole("mpole_k6")

def test_mpole_k6_tilt():
    run_a_multipole("mpole_k6_tilt")
