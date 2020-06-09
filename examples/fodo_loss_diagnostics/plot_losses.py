#!/usr/bin/env python

from __future__ import print_function
import sys
import os
import numpy as np
import matplotlib.pyplot as plt

import h5py

h5 = h5py.File("loss_diagnostics.h5", 'r')

# s is the longitudinal position of the loss
s = np.array(h5.get("s"))

# coords is the phase space coordinates of the lost particle including particle ID
coords = h5.get("coordinates")

print("s.shape: ", s.shape)
print("coords.shape: ", coords.shape)

ax1 = plt.subplot(211)
ax1.plot(s, coords[:, 0], '.', label='x losses')
xticklabels1 = ax1.get_xticklabels()
plt.ylabel('x loss position')
plt.setp(xticklabels1, visible=False)
plt.legend(loc='best')

ax2 = plt.subplot(212, sharex=ax1)
ax2.plot(s, coords[:, 2], '.', label='y losses')
plt.xlabel('s position [m]')
plt.ylabel('y loss position')
plt.legend(loc='best')

plt.subplots_adjust(hspace=.001)

plt.show()

h5.close()
