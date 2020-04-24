import os
import sys

with open(os.path.join(sys.argv[1], "output/TIME_MEASUREMENT_dmesg.txt"), 'r') as f:
    time_sum = 0
    for i in f.readlines():
        tmp = i.strip().split()
        time_sum += float(tmp[4]) - float(tmp[3])

time_unit = time_sum / 5000
print(time_unit)

with open(os.path.join(sys.argv[1], "output/{}_dmesg.txt".format(sys.argv[2])), 'r') as f:
    for i in f.readlines():
        tmp = i.strip().split()
        print(tmp[2], (float(tmp[4]) - float(tmp[3])) / time_unit)

