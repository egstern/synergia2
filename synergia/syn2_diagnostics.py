#!/usr/bin/env python
import numpy
import numpy.linalg
from math import sqrt
import sys
import tables
from collections import deque
from mpi4py import MPI
import time

import s2_fish.s2_diagnostics as s2_diagnostics

x = 0
xprime = 1
y = 2
yprime = 3
z = 4
zprime = 5

x_xprime = 0
x_y = 1
xprime_y = 2
x_yprime = 3
xprime_yprime = 4
y_yprime = 5
x_z = 6
xprime_z = 7
y_z = 8
yprime_z = 9
x_zprime = 10
xprime_zprime = 11
y_zprime = 12
yprime_zprime = 13
z_zprime = 14

xprime_x = 0
y_x = 1
y_xprime = 2
yprime_x = 3
yprime_xprime = 4
yprime_y = 5
z_x = 6
z_xprime = 7
z_y = 8
z_yprime = 9
zprime_x = 10
zprime_xprime = 11
zprime_y = 12
zprime_yprime = 13
zprime_z = 14

def get_labframe_means(bunch):
    means = numpy.zeros([3],numpy.float64)
    s2_diagnostics.get_spatial_means(bunch.get_store(),means)
    means[0] /= bunch.units[0] 
    means[1] /= bunch.units[2]  
    means[2] /= -bunch.units[4]   
    return means

def get_spatial_means_stds(bunch):
    means = numpy.zeros([3],numpy.float64)
    stds = numpy.zeros([3],numpy.float64)
    s2_diagnostics.get_spatial_means_stds(bunch.get_store(),means,stds)
    return means,stds

def get_spatial_minmax(bunch):
    bmin = numpy.zeros([3],numpy.float64)
    bmax = numpy.zeros([3],numpy.float64)
    s2_diagnostics.get_spatial_minmax(bunch.get_store(),bmin,bmax)
    return bmin, bmax

def old_get_diagnostics(bunch):
    means = numpy.zeros([6],numpy.float64)
    stds = numpy.zeros([6],numpy.float64)
    for i in range(0,6):
        means[i] = numpy.average(bunch[i,:])
    mom2s = numpy.zeros([6,6],numpy.float64)
    corrs = numpy.zeros([6,6],numpy.float64)
    for i in range(0,6):
        for j in range(i,6):
            tmp = bunch[i,:]*bunch[j,:]
            mom2s[i,j] = numpy.average(tmp) - means[i]*means[j]
            mom2s[j,i] = mom2s[i,j]
    for i in range(0,6):
        stds[i] = sqrt(mom2s[i,i])
    for i in range(0,6):
        for j in range(i,6):
            corrs[i,j] = mom2s[i,j]/(stds[i]*stds[j])
            corrs[j,i] = corrs[i,j]
    return means,stds,mom2s,corrs

def get_diagnostics(bunch,units):
    means = numpy.zeros([6],numpy.float64)
    mom2s = numpy.zeros([6,6],numpy.float64)
    corrs = numpy.zeros([6,6],numpy.float64)
    diagmom4s = numpy.zeros([6],numpy.float64)
    s2_diagnostics.get_moments_corrs(bunch.get_store(),units,means,mom2s,corrs,diagmom4s)
    return means,mom2s,corrs,diagmom4s

class Diagnostics:
    def __init__(self,units,short=False,save_period=100):
        self.s = []
        self.means = []
        self.mom2s = []
        self.corrs = []
        self.diagmom4s = []
        self.stds = []
        self.emitxs = []
        self.emitys = []
        self.emitzs = []
        self.emitxys = []
        #~ self.emitxzs = []
        self.emitxyzs = []
        self.u = units
        self.n = []
        self.short = short
        self.save_period=save_period

    def add(self,s,bunch):
        means,mom2s,corrs,diagmom4s = get_diagnostics(bunch,self.u)
        self.s.append(s)
        self.means.append(means)
        if not self.short:
            self.mom2s.append(mom2s)
            self.corrs.append(corrs)
            self.diagmom4s.append(diagmom4s)
        #derived quantities
        self.stds.append(numpy.sqrt(numpy.diagonal(mom2s)))
        self.emitxs.append(sqrt(abs(numpy.linalg.det(mom2s[0:2,0:2])))*self.u[xprime])
        self.emitys.append(sqrt(abs(numpy.linalg.det(mom2s[2:4,2:4])))*self.u[yprime])
        self.emitzs.append(sqrt(abs(numpy.linalg.det(mom2s[4:6,4:6])))*self.u[zprime])
        self.emitxys.append(sqrt(abs(numpy.linalg.det(mom2s[0:4,0:4])))\
                        *self.u[xprime]*self.u[yprime])
        self.emitxyzs.append(abs(sqrt(abs(numpy.linalg.det(mom2s)))\
                        *self.u[xprime]*self.u[yprime]*self.u[zprime]))
        self.n.append(bunch.total_num)
        if self.save_period:
                if (len(self.n) % self.save_period == 0) and \
                    (MPI.COMM_WORLD.Get_rank() == 0):
                    t0 = time.time()
                    self.write_hdf5("tmp-diagnostics")
    
    def get_s(self):
        return numpy.array(self.s)
    def get_means(self):
        return numpy.array(self.means)
    def get_stds(self):
        return numpy.array(self.stds)
     
    def write(self,filename_prefix):
        if MPI.COMM_WORLD.Get_rank() == 0:
            if self.short:
                raise RuntimeError, \
                    "Diagnostics: write not supported for short=True"
            # same format as fort.24, et. al, but we don't calculate Twiss alpha
            fx = open(filename_prefix+"_x.dat","w")
            fy = open(filename_prefix+"_y.dat","w")
            fz = open(filename_prefix+"_z.dat","w")
            femit = open(filename_prefix+"_emit.dat","w")
            fcorr = open(filename_prefix+"_corr.dat","w")
            for i in range(0,len(self.s)):
                fx.write("%g %g %g %g %g 0.0 %g\n" % \
                        (self.s[i],
                        self.means[i][x], self.stds[i][x],
                        self.means[i][xprime], self.stds[i][xprime],
                        self.emitxs[i]))
                fy.write("%g %g %g %g %g 0.0 %g\n" % \
                        (self.s[i],
                        self.means[i][y], self.stds[i][y],
                        self.means[i][yprime], self.stds[i][yprime],
                        self.emitys[i]))
                fz.write("%g %g %g %g %g 0.0 %g\n" % \
                        (self.s[i],
                        self.means[i][z], self.stds[i][z],
                        self.means[i][zprime], self.stds[i][zprime],
                        self.emitzs[i]))
                femit.write("%g %g %g %g %g %g\n" % \
                        (self.s[i],
                        self.emitxs[i],self.emitys[i],self.emitzs[i],
                        self.emitxys[i],self.emitxyzs[i]))
                for ii in range(0,6):
                    for jj in range(ii+1,6):
                        fcorr.write("%g" % self.corrs[i][ii,jj])
                        if not (ii==5 and jj==5):
                            fcorr.write(" ")
                fcorr.write("\n")
            fx.close()
            fy.close()
            fz.close()
            femit.close()
            fcorr.close()

    def write_hdf5(self,filename_prefix,compress_level=1):
        if MPI.COMM_WORLD.Get_rank() == 0:
            f = tables.openFile(filename_prefix+".h5",mode = "w")
            # n.b. filter (and compress_level) not (yet) used
           # filter = tables.Filters(complevel=compress_level)
            root = f.root
            hdfarray = f.createArray(root,'s',numpy.array(self.s),"position")
            hdfarray = f.createArray(root,'mean',numpy.array(self.means),"centroid")
            if not self.short:
                hdfarray = f.createArray(root,'mom2',numpy.array(self.mom2s),"second moments")
                hdfarray = f.createArray(root,'corr',numpy.array(self.corrs),"correlation coefficients")
                hdfarray = f.createArray(root,'diagmom4',numpy.array(self.diagmom4s),"fourth moments on diagonal")
            hdfarray = f.createArray(root,'std',numpy.array(self.stds),"standard deviation")
            hdfarray = f.createArray(root,'emitx',numpy.array(self.emitxs),"x emittance")
            hdfarray = f.createArray(root,'emity',numpy.array(self.emitys),"y emittance")
            hdfarray = f.createArray(root,'emitz',numpy.array(self.emitzs),"z emittance")
            hdfarray = f.createArray(root,'emitxy',numpy.array(self.emitxys),"x-y emittance")
            hdfarray = f.createArray(root,'emitxyz',numpy.array(self.emitxyzs),"x-y-z emittance")
            hdfarray = f.createArray(root,'units',numpy.array(self.u),"units")
            # octave gets confused if n is not an array of doubles
            hdfarray = f.createArray(root, 'n',numpy.array(self.n,'d'),"n") 
            f.close()

class Stored_means:
    def __init__(self,units,maxque_len=10):
        self.units=units
        self.maxque_len=maxque_len
        self.s = deque()
        self.ibucket=deque()
        self.means = deque()
        self.bunch_np=deque()
   
   
    def add(self,s,bunch):
        if self.maxque_len==0: return
        que_len=len(self.s)
        if que_len >= self.maxque_len:
             self.s.pop()
             self.ibucket.pop()
             self.means.pop()
             self.bunch_np.pop()
        self.s.appendleft(s)
        self.ibucket.appendleft(bunch.bucket_num)
        means = get_labframe_means(bunch)
        self.means.appendleft(means)
        self.bunch_np.appendleft(bunch.bunch_np)
        
    
    def get_s(self):
        return numpy.array(self.s)
    
    def get_ibucket(self):
        return numpy.array(self.ibucket)
    
    def get_means(self):
        return numpy.array(self.means)
    
    def get_bunchnp(self):
        return numpy.array(self.bunch_np)