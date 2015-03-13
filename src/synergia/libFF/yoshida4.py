#!/usr/bin/env python

from decimal import Decimal, getcontext

getcontext().prec=30
third2 = Decimal(2)**(Decimal(1)/Decimal(3))
c1 = c4 = Decimal(1)/(Decimal(2)*(Decimal(2)-third2))
c2 = c3 = (Decimal(1) - third2)/(Decimal(2)*(Decimal(2)-third2))
d1 = d3 = Decimal(1)/(Decimal(2)-third2)
d2 = -third2/(Decimal(2)-third2)

print '    const double c1 = c4 =' + str(c1) + ';'
print '    const double c2 = c3 =' + str(c2) + ';'
print '    const double d1 = d3 =' + str(d1) + ';'
print '    const double d2 =' + str(d1) + ';'
