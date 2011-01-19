import sys
sys.path.append('..')
sys.path.append('../../foundation')
sys.path.append('../../convertors')

from mpi4py import MPI
from foundation import Reference_particle, Four_momentum
from bunch import Bunch
from bunch import Diagnostics_track, Diagnostics_writer
import convertors
import numpy
from nose.tools import *

mass = 2.2;
total_energy = 3.0
total_num = 100
real_num = 1.0e12
proton_charge = 1
turns = 17;
turn_length = 246.8;
partial_s = 123.4;


reference_particle = Reference_particle(proton_charge, mass, total_energy)
reference_particle.set_trajectory(turns, turn_length, partial_s)
comm = MPI.COMM_WORLD
bunch = Bunch(reference_particle, total_num, real_num, comm)
particles = bunch.get_local_particles()
particles[:, 0:6] = numpy.random.lognormal(size=[bunch.get_total_num(), 6])

def test_construct():
    diagnostics = Diagnostics_track(0)

def test_construct2():
    diagnostics = Diagnostics_track(bunch, 0)

def test_update():
    diagnostics = Diagnostics_track(bunch, 0)
    diagnostics.update(bunch)

def test_create_writer():
    diagnostics = Diagnostics_track(bunch, 0)
    diagnostics_writer = Diagnostics_writer("test_create_writer_track.h5",
                                             diagnostics)
