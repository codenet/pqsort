#!/usr/bin/python -o

import filecmp
import fileinput
import sys

numThreads = [1, 2, 4, 8, 7, 100]
times = []
for line in fileinput.input("time.txt"):
    time = float(line)
    if (time == 0):
        print "Crazy: Serial time = 0"
        sys.exit(0)
    else:
        times.append(time)

for iter in range(len(times)):
    speedup = times[0]/times[iter]
    print "Speedup for " + str(numThreads[iter]) + " threads = " + str(speedup)

    


