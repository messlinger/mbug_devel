
from mbug_base import *

#==========================================================================
class mbug_2820(mbug):
    """ Represents an MBUG-2820 Environmeter device (Temperature & Humidity)"""

    _type = 2820

    def __init__(self, serial=None):
        mbug.__init__(self, self._type, serial)

    def read_raw(self):
        """ Get the current measurement raw data. """      
        self._write( (0xA0, 0x00, 0x00) )   # Read, raw format 
        d = self._read()
        raw = d[0]+(d[1]<<8)+(d[2]<<16)+(d[3]<<24)
        if (raw==0xffffffff): raise Exception('MBUG-2820 Sensor Error')
        return raw

    def from_raw(self, raw):
        if (raw==0xffffffff): raise Exception('MBUG-2820 Sensor Error')
        rH = ((raw>>16) & 0x3fff) * 100./2**14
        T = ((raw>>2) & 0x3fff) * 165./2**14 - 40
        return T, rH
        
    def read(self):
        """ Get the current measurement in physical units.
        Temperature in degrees Celsius, rel. humidity in percent. """
        self._write((0xA0, 0x00, 0x01))     # Read, ascii format, 1st part (temperature)
        s1 = self._read().strip('\0')
        self._write((0xA0, 0x01, 0x01))     # Read, ascii format, 2nd part (humidity)        
        s2 = self._read().strip('\0')
        try: T, rH = float(s1), float(s2)
        except:
            raise Exception('MBUG-2120 Device Error (Data: %s,%s)'%(s1,s2))
        if T<=-274 or rH<0: raise Exception('MBUG-2120 Sensor Error')
        return T, rH
    
#==========================================================================
def mbug_2820_test():
    # Print device list
    devs = list_devices(2820)
    print "Attached MBUG-2820 devices:"
    print devs if devs!=[] else 'None'

    # Open devices
    for ser in devs:
        try:
            print "Device", ser
            dev = mbug_2820(ser)
            ##data = dev.read_raw()
            ##T, rH = dev.from_raw(data)
            ##print "Data:", data, T, rH
            T,rH = dev.read()
            print "Data:", T, rH
        finally:
            try: dev.close()
            except: pass

#--------------------------------------------------------------------------    
if __name__ == "__main__":
   mbug_2820_test()
   pass


