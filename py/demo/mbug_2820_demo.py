#
# Demo script for the MBUG_2820 device interface class
#

try: import mbug
except ImportError: import mbug_dev as mbug
import time

# Open the first device and print the current temperature
dev = mbug.mbug_2820()

# Open file
file = open('measurement.dat', 'a', 0)

# Read until break
while(1):
    T,rH = dev.read()
    t = time.time()
    print t, T, rH
    file.write("%.2f\t%.3f\t%.2f\n" % (t, T, rH) )
    file.flush()
    time.sleep(2)
    

