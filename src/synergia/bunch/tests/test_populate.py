#!/usr/bin/env python

import sys
sys.path.append('..')
sys.path.append('../../foundation')
sys.path.append('../../convertors')

from mpi4py import MPI
from foundation import Reference_particle, Four_momentum, Distribution, \
    Random_distribution
from bunch import Bunch, Fixed_t_z_zeroth, Fixed_t_z_ballistic, populate_6d, \
    populate_transverse_gaussian
import convertors
import numpy
from nose.tools import *

mass = 2.2;
total_energy = 3.0
total_num = 100
real_num = 1.0e12
proton_charge = 1
reference_particle = Reference_particle(proton_charge, mass, total_energy)
seed = 987654321

def test_6d():
    bunch = Bunch(reference_particle, total_num, real_num,
               MPI.COMM_WORLD)
    distribution = Random_distribution(seed, MPI.COMM_WORLD)
    means = numpy.zeros((6,), 'd')
    covariances = numpy.zeros((6, 6), 'd')
    for i in range(0, 6):
        covariances[i][i] = 1.1 * i
    populate_6d(distribution, bunch, means, covariances)

def test_transverse_gaussian():
    bunch = Bunch(reference_particle, total_num, real_num,
               MPI.COMM_WORLD)
    distribution = Random_distribution(seed, MPI.COMM_WORLD)
    means = numpy.zeros((6,), 'd')
    covariances = numpy.zeros((6, 6), 'd')
    for i in range(0, 6):
        covariances[i][i] = 1.1 * i
    cdt = 3.14
    populate_transverse_gaussian(distribution, bunch, means, covariances, cdt)
