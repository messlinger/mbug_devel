
from .mbug_base import *

#==========================================================================
class mbug_2818(mbug):
    """ Represents an MBUG-2818 thermometer device (8 channel)"""

    _type = 2818

    def __init__(self, serial=None):
        mbug.__init__(self, self._type, serial)

    def read_raw(self):
        """ Get the raw adc readings. """      
        self._write( (0xFA, 0xA0) )         # Raw format, read
        d = self._read()
        raw = [float(d[i]+(d[i+1]<<8)) for i in range(0,16,2)]
        return raw

    def read_ascii(self):
        """ Get the current reading as temperature"""      
        self._write( (0xFB, 0xA0) )     # ASCII format, read
        raw = self._read()
        return [float(i.strip()) for i in raw.strip(b'\0').split(b',')]
        
    read = read_ascii

    def set_mode(self, mode):
        """Set the acquisition mode:
        'inst': Use last measured value if not read before, else wait for next.
        'last': Always use last valid value, never block.
        'next': Always wait for next incoming value.
        'sync': Trigger a new acquisition when reading."""
        try: cmd = {'INST':0xF2, 'LAST':0xF3, 'NEXT':0xF4, 'SYNC':0xF5}[str(mode).upper()]
        except KeyError: raise ValueError('Invalid mode')
        self._write( (cmd,) )
        return

    def set_dout(self, bits):
    	""" Set the digital output state. 
    	    Bits can be passed as list or integer."""
    	try: raw = sum([ bool(bit)<<i for (i,bit) in enumerate(bits) ])
    	except TypeError: raw = bits
    	self._write( (0xE0, raw&0xFF, 0 ) )
    	return
    
    def set_tris(self, bits):
        """ Set the gpio tristate state (1:Analog Input 0:Digital Output).
        Bits can be passed as list or integer."""
        try: raw = sum([ bool(bit)<<i for (i,bit) in enumerate(bits) ])
        except TypeError: raw = int(bits)    # Might be packed integer
        self._write( (0xE2, raw&0xFF, 0 ) )
        return

    def enable_pwm(self, channels=1):
        """ Enable the pwm output on channel 5. """
        try: chb = sum([ bool(bit)<<i for (i,bit) in enumerate(channels) ]) # try to iterate
        except TypeError: chb = int(channels)
        self._write( (0xE4, chb%256, 0 ) )
        return

    def set_pwm(self, duty):
        """ Set the pwm duty cycle. Duty cycle resolution is 10 bit.
        Duty cycle can be passed as integer in range 0..1023 or
        as float in range 0.1..1.0
        For upward compatibilty, duty can also passed as list. """
        try: duty0 = duty[0] # try to iterate
        except TypeError: duty0 = duty
        if isinstance(duty0, float): duty0 = duty0*1024
        duty0 = int(duty0)
        if duty0<0: duty0 = 0
        if duty0>1023: duty0 = 1023
        self._write( (0xE5, 0, duty0%256, (duty0/256)&0xFF ) )
        return

#==========================================================================

def mbug_2818_test():
    # Print device list
    devs = list_devices(2818)
    print("Attached MBUG-2818 devices:")
    print(devs if devs!=[] else 'None')

    # Open devices
    for ser in devs:
        try:
            print("Device", ser)
            dev = mbug_2818(ser)
            temps = dev.read()
            print("Temperatures: ", temps)
        finally:
            try: dev.close()
            except: pass

#---------------------------------------------------------------------------
if __name__ == "__main__":
    mbug_2818_test()

