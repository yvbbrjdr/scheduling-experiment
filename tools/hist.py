#!/usr/bin/env python3

import sys
diff = []

if len(sys.argv) < 2:
    print("Use as hist.py file bins width")
    sys.exit(0)
with open(sys.argv[1], 'r') as file:
    for f in file:
        split = f.split(':')
        diff.append(float(split[1]))

def bin(arr, size):
    width = float(sys.argv[3]) / size # (max(arr) - min(arr)) / size
    count = [0 for i in range(0,size + 1)]

    for item in arr:
        count[int( item  // width )] += 1

    return count

print("Min time:", min(diff))
print("Max time:", max(diff))

print("Histogram binned with " + str(sys.argv[2]) + " bins")

print("Bins (minimum value & inclusive):")
print([i * float(sys.argv[3])/int(sys.argv[2]) for i in range(int(sys.argv[2]))])

print("Binned values:")
print(bin(diff, int(sys.argv[2])))
