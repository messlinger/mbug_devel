#
# Demo script for the MBUG_2810 device interface class
#

try: import mbug
except ImportError: import mbug_dev as mbug
import time

# Open the first device and print the current temperature
therm = mbug.mbug_2818()

# Open file
file = open('temperature.dat', 'a', 0)

# Read until break
while(1):
    T = therm.read()[0]
    t = time.time()
    print t, T
    file.write("%.2f\t%.3f\n" % (t, T) )
    file.flush()
    time.sleep(10)
    

