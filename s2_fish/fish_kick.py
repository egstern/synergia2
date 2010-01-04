#!/usr/bin/env python

#from macro_bunch import get_longitudinal_period_size
from impedance import *
from space_charge import *
from s2_containers import *
#from s2_solver_transverse_fftw import *
from pardebug import pardebug

from mytimer import mytimer
import constraints

import time
import synergia
import sys

from mpi4py import MPI
#import numpy
#import math


def listify(x):
    if type(x) == type([]) or type(x) == type(()):
        return x
    else:
        return [x]


counter = 0   

def apply_kick(mbunch_in,tau, space_charge=None,impedance=None, aperture=None):
    
#    if not (space_charge or impedance) : return
    
    global counter 
    counter += 1
    show_timings=1
    # ONLY FOR DEBUG PURPOSES
    #if counter > 2:
    #	return
       
    mbunches = listify(mbunch_in) 
    
    for bunch_index, mbunch in enumerate(mbunches):    
        if aperture:
            constraints.apply_circular_aperture(mbunch.get_store(),aperture)
            mytimer("apply aperture")       
        mbunch.convert_to_fixedt()
        mytimer("convert")
	
        if mbunch.periodic:	
            length=mbunch.get_longitudinal_period_size()
            constraints.apply_longitudinal_periodicity(mbunch.get_store(),length)
            mytimer("apply periodicity")
        
	    
        if space_charge:
            apply_space_charge_kick(mbunch,space_charge,tau)   
	    
        if impedance: 
            apply_impedance_kick(mbunch,impedance,tau, bunch_index) 
        mbunch.convert_to_fixedz()
        mytimer("unconvert")


	
