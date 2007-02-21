#!/usr/bin/env python

from s2_fish import *
from macro_bunch import Macro_bunch
import numarray
import time
import sys
import math
from math import pi

import unittest

def potential_uniform_sphere(Q,r0,rvec):
    """ The potential due to a uniform sphere of radius r0 and charge
    Q, centered at the origin."""
    r = math.sqrt(rvec[0]**2 + rvec[1]**2 + rvec[2]**2)
    if r <= r0:
        retval = Q/(8*pi*r0**3)*(3*r0**2-r**2)
    else:
        retval = Q/(4*pi*r)
    return retval

def compare_on_axis(axis,shape,size,offset,phi,Q,r0):
    r = numarray.arrayrange(shape[axis])*size[axis]/(shape[axis]-1) \
        - 0.5*size[axis] + offset[axis]
    phi_r = numarray.zeros(shape[axis],numarray.Float)
    exact = numarray.zeros(shape[axis],numarray.Float)
    max_err = 0.0
    sum_err = 0.0
    for i in range(0,shape[axis]):
        rvec = [0.0,0.0,0.0]
        ri = r[i]
        if abs(ri - 0.5*size[axis] - offset[axis]) < 1.0e-10:
            ri = 0.5*size[axis] + offset[axis]- 1.0e-10
        rvec[axis] = ri
        phi_r[i] = phi.get_val(rvec)
        exact[i] = potential_uniform_sphere(Q,r0,rvec)
        abs_err = abs(1.0 - phi_r[i]/exact[i])
        if abs_err > max_err:
            max_err = abs_err
        sum_err += abs_err
    mean_err = sum_err/shape[axis]
    return r,phi_r,exact,max_err,mean_err

class Test_solver_fft_open(unittest.TestCase):    
    def test_01_fft(self):
        max_err = fft_tester(8,32,64);
        self.assertAlmostEqual(max_err,0.0)
        
    def test_02_rough_grid(self):
        shape = (16,16,16)
        size = (2.0,2.0,2.0)
        offset = (0.0,0.0,0.0)
        sf = Real_scalar_field(shape,size,offset)
        mb = Macro_bunch()
        Q = 100000
        r0 = 0.2
        mb.init_sphere(Q,r0)
        total_charge = deposit_charge_cic(sf,mb.store)
        phi = solver_fft_open(sf)
        for axis in range(0,3):
            r,phi_r,exact,max_err,mean_err = compare_on_axis(axis,shape,size,
                                                             offset,phi,Q,r0)
            self.failIf(max_err>0.07)
            self.failIf(mean_err>0.02)

    def test_03_fine_grid(self):
        shape = (48,48,48)
        size = (2.0,2.0,2.0)
        offset = (0.0,0.0,0.0)
        sf = Real_scalar_field(shape,size,offset)
        mb = Macro_bunch()
        Q = 100000
        r0 = 0.2
        mb.init_sphere(Q,r0)
        total_charge = deposit_charge_cic(sf,mb.store)
        phi = solver_fft_open(sf)
        for axis in range(0,3):
            r,phi_r,exact,max_err,mean_err = compare_on_axis(axis,shape,size,
                                                             offset,phi,Q,r0)
            self.failIf(max_err>0.015)
            self.failIf(mean_err>0.003)

    def test_04_asymmetric_grid(self):
        shape = (16,24,32)
        size = (2.0,5.0,3.0)
        offset = (0.1,0.5,0.4)
        sf = Real_scalar_field(shape,size,offset)
        mb = Macro_bunch()
        Q = 100000
        r0 = 0.2
        mb.init_sphere(Q,r0)
        total_charge = deposit_charge_cic(sf,mb.store)
        phi = solver_fft_open(sf)
        max_tolerance = [0.07,0.05,0.09]
        mean_tolerance = [0.03,0.006,0.03]
        for axis in range(0,3):
            r,phi_r,exact,max_err,mean_err = compare_on_axis(axis,shape,size,
                                                             offset,phi,Q,r0)
            self.failIf(max_err>max_tolerance[axis])
            self.failIf(mean_err>mean_tolerance[axis])

if __name__ == '__main__':
    unsuccessful = 0
    solver_suite = unittest.TestLoader().loadTestsFromTestCase(Test_solver_fft_open)
    retval = unittest.TextTestRunner(verbosity=2).run(solver_suite)
    if not retval.wasSuccessful():
        unsuccessful = 1

    sys.exit(unsuccessful)
