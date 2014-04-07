
from .mbug_base import *

#==========================================================================
class mbug_2811(mbug):
    """ Represents an MBUG-2811 thermometer device """

    _type = 2811

    def __init__(self, serial=None):
        mbug.__init__(self, self._type, serial)

    def read_raw(self):
        """ Get the current temperature in degrees Celsius as float. """      
        self._write( (0xFA, 0xA0, 0xF2) )   # Raw format (0xF2 is the old revisions read cmd)
        d = self._read()
        raw = d[0]+(d[1]<<8)
        return raw
        
    def read_ascii(self):
        """ Get the current temperature in degrees Celsius as float. """      
        self._write('read\0')     # Ascii format 
        s = self._read()
        return float(s.strip(b'\0').splitlines()[0])

    read = read_ascii    
    
#==========================================================================
def mbug_2811_test():
    # Print device list
    devs = list_devices(2811)
    print("Attached MBUG-2811 devices:")
    print(devs if devs!=[] else 'None')

    # Open devices
    for ser in devs:
        try:
            print("Device", ser)
            dev = mbug_2811(ser)
            temp = dev.read()
            print("Temperature:", temp)
        finally:
            try: dev.close()
            except: pass

#--------------------------------------------------------------------------    
if __name__ == "__main__":
    mbug_2811_test()

