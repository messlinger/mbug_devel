
from .mbug_base import *
import time as _time

#==========================================================================
class mbug_1220(mbug):
    """ Represents an MBUG-1220 Plant-O-Mat device (Bastelversuch 2014) """

    _type = 1220

    def __init__(self, serial=None):
        mbug.__init__(self, self._type, serial)

    def read(self):
        """ Get the current measurement. """      
        self._write( (0xA0, ) ) 
        _time.sleep(0.8)  # Measurement takes 1 second, usb timeout is 1 second 
        d = self._read()
        counter = d[0]+(d[1]<<8)+(d[2]<<16)+(d[3]<<8)
        level = d[6]
        return counter, level

    def set_pump( self, interval=10000 ):
        """Switch pump on for <interval> milliseconds."""
        if (interval>2**16-1): interval = 2**16-1
        self._write( (0xB0, 0x01, interval&0xFF, (interval>>8)&0xFF) )
        return
       
    def set_speaker( self, interval=100 ):
        """Switch speaker on for <interval> milliseconds, modulate with <modinterval> milliseconds"""
        if (interval>2**16-1): interval = 2**16-1
        self._write( (0xB0, 0x02, interval&0xFF, (interval>>8)&0xFF ) ) 
        return
    
    def set_led( self, led1, led2 ):
        """Turn leds on/off"""
        led1 = 1 if led1 else 0
        led2 = 1 if led2 else 0
        self._write( (0xB0, 0x03, led1, led2 ) )
        return
    
#==========================================================================


