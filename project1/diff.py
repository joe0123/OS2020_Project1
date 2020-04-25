import sys
import numpy as np

with open("./output/TIME_MEASUREMENT_dmesg.txt", 'r') as f:
    time_sum = 0
    for i in f.readlines():
        tmp = i.strip().split()
        time_sum += float(tmp[-1]) - float(tmp[-2])

time_unit = time_sum / 5000

pid2name = dict()
with open("./output/{}_stdout.txt".format(sys.argv[1]), 'r') as f:
    for i in f.readlines():
        tmp = i.strip().split()
        pid2name[int(tmp[1])] = tmp[0]

name_real = []
time_real = []
with open("output/{}_dmesg.txt".format(sys.argv[1]), 'r') as f:
    for i in f.readlines():
        tmp = i.strip().split()
        name_real.append(pid2name[int(tmp[-3])])
        time_real.append([float(tmp[-2]), float(tmp[-1])])
time_real = np.array(time_real)
time_real = time_real - np.min(time_real)

name_theo = []
time_theo = []
with open("./my_output/{}_theo.txt".format(sys.argv[1]), 'r') as f:
    for i in f.readlines():
        tmp = i.strip().split()
        name_theo.append(tmp[0])
        time_theo.append([float(tmp[1]), float(tmp[2])])
time_theo = np.array(time_theo)
time_theo = (time_theo - np.min(time_theo)) * time_unit

with open("./my_output/{}_diff.txt".format(sys.argv[1]), 'w') as f:
    for i in range(len(name_real)):
        if name_real[i] != name_theo[i]:
            f.write("Different from theory!")
            break
        else:
            f.write("{} {} {} {} {}\n".format(name_real[i], time_real[i][0], time_real[i][1], time_theo[i][0], time_theo[i][1]))



