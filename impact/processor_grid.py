#!/usr/bin/env python

from mpi4py import MPI
have_impact = True
try:
    from UberPkgpy import *
except:
    have_impact = False

class Processor_grid:
    def __init__(self,columns):
        self.pgrid2d = Pgrid2d()
        self.npcol = columns
        self.nprow = MPI.COMM_WORLD.Get_size()/self.npcol
        mpi_comm_world = fmpi_comm_world_external()
        construct_Pgrid2d_external(self.pgrid2d, fmpi_comm_world_external(),
                                   self.nprow, self.npcol)

    def get_pgrid2d(self):
        return self.pgrid2d
    def get_nrow(self):
        return self.nprow
    def get_ncol(self):
        return self.npcol
