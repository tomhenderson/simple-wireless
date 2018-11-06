#/usr/bin/env python2
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import sys

f = open(sys.argv[1])
x = []
y = []
for line in f:
    columns = line.split()
    x.append (columns[0])
    y.append (columns[1])
f.close()
plt.plot(x, y)
plt.title(sys.argv[2])
plt.savefig('link-performance-rssi.pdf')
