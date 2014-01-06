
from mbug_base import *
import mbug_2151_target
import time as _time

#==========================================================================
class mbug_2151(mbug):
    """ Represents an MBUG-2151 433MHz transmitter"""
    
    _type = 2151

    def __init__(self, serial=None):
        mbug.__init__(self, self._type, serial)
        self._base_clock = self.get_base_clock()
        self.target._dev = self

    def _iocmd(self, cmd):
        self._write( cmd )
        ret = self._read()
        status = (ret[0] + ret[1]*256) - 2**16*(ret[1]&0x80>0)   # signed 16bit
        if status!=0:
            errmsg = {-1: 'Unknown command',
                }.get( status, 'Undefined error' )
            raise RuntimeError('mbug_2165: Communication Error: %s (%d)' % (errmsg,status) )
        return ret[2:]

    def set_sequence_bitstream(self, data):
        """ Set a transmission byte sequence in bitstream mode. The data will be 
        transmitted bit per bit, starting with the least significant bit of the 
        first byte. The maximum length is 255 bytes at the moment. """
        nbytes = len(data)
        if nbytes>255: raise ValueError('Maximum sequence length is 255.')
        s = self._size_out-4 # max data size per packet
        # First packet: set_sequence
        d, data = data[:s], data[s:]
        self._iocmd( bytearray(( 0xB0, 0x00, 0x00, len(d) )) + bytearray(d) )
        #Subsequent packets: append_sequence
        while (len(data)>0):
            d, data = data[:s], data[s:]
            self._iocmd( bytearray(( 0xB0, 0x00, 0x01, len(d) )) + bytearray(d) )
        return
        
    def set_sequence_times(self, data, unit='ms', clock_div=0, bits=0):
    	""" Set a transmission sequence in timed mode. The sequence is interpreted 
    	as consecutive on/off intevalls (starting with on). The default unit is
    	milliseconds (can be changed with the unit parameter). Suitable clock
    	dividers and word length (8 or 16 bit) are chose automatically from the 
    	data if not passed explicitely. If set manually, the on/off time intervals
    	equate as t*clkdiv/base_clock. Minimum interval = ??? XXXX"""
    	print ":TODO: NOT IMPLEMENTED YET"

    def set_sequence_times_8(self, data):
        nbytes = len(data)
        if nbytes>255: raise ValueError('Maximum sequence length is 255.')
        s = self._size_out-4 # max data size per packet
        # First packet: set_sequence
        d, data = data[:s], data[s:]
        self._iocmd( bytearray(( 0xB0, 0x00, 0x10, len(d) )) + bytearray(d) )
        #Subsequent packets: append_sequence
        while (len(data)>0):
            d, data = data[:s], data[s:]
            self._iocmd( bytearray(( 0xB0, 0x00, 0x01, len(d) )) + bytearray(d) )
        return

    def set_sequence_times_16(self, data):
        nitems = len(data)
        if nitems>127: raise ValueError('Maximum sequence length is 127.')
        bytes = []
        for item in data: bytes += [ item&0xFF, (item>>8)&0xFF]
        s = self._size_out-4 # max data size per packet
        # First packet: set_sequence
        d, bytes = bytes[:s], bytes[s:]
        self._iocmd( bytearray(( 0xB0, 0x00, 0x20, len(d) )) + bytearray(d) )
        #Subsequent packets: append_sequence
        while (len(bytes)>0):
            d, bytes = bytes[:s], bytes[s:]
            self._iocmd( bytearray(( 0xB0, 0x00, 0x01, len(d) )) + bytearray(d) )
        return

    def set_sequence(self, data, seq_type):
        """Set a transmission sequence of the type specified by seq_type
        ('bitstream', 'timed_8', 'timed_16')."""
        if seq_type=='bitstream':
            return self.set_sequence_bitstream(data)
        elif seq_type.startswith('timed_8'):
            return self.set_sequence_times_8(data)
        elif seq_type.startswith('timed_16'):
            return self.set_sequence_times_16(data)
        else: raise ValueError('Invalid sequence type')
    
    def set_iterations(self, niter):
        """ Set the number of transmission iterations.
        0: infinite, Maximum: 2**16-1 at the moment. """
        if niter=='inf': niter=0
        if niter<0 or niter>=1<<16:  raise ValueError('Invalid number of iterations.')
        self._iocmd( (0xB0, 0x01, niter&0xFF, (niter>>8)&0xFF, 0x00, 0x00 ) )
        return

    def set_clock_div(self, div):
        """ Set the clock divider. Bitrate = base_clock/div, Time = base_time*div
        Minimum div in bitstream mode is 10, maximum div is 2**16-1. """
        div = int(div)
        if div<=0 or div>=1<<16:  raise ValueError('Invalid number of iterations.')
        self._iocmd( (0xB0, 0x02, div&0xFF, (div>>8)&0xFF, 0x00, 0x00 ) )
        return

    def set_bitrate(self, freq):
        """ Set the transmission rate (in Hz, via clock div). Minimum rate is 61sec.
        Maximum rate is 400kHz for bitstream mode, 4Mhz for timed modes.
        Returns the actually set bitrate."""
        if freq<=0: raise ValueError('Invalid bitrate.')
        div = round(1.*self._base_clock/freq)
        if div>2**16-1: div=2**16-1;
        rfreq = self._base_clock/div
        if abs(1.*rfreq/freq-1.)>0.01:  # Deviation > 1%
            print "Warning. Set bitrate to %.1e."%(rfreq)
        self.set_clock_div(div)
        return rfreq

    def set_timebase(self, interval):
        """ Set the transmission base interval (in seconds, via clock div).
        Minimum interval is 2.5us for bitstream mode, 0.25us for timed modes.
        Maximum base interval is 16.38375ms. Resolution is 0.25us.
        Returns the actually set interval."""
        div = round(1.*self._base_clock*interval)
        if div>2**16-1: div=2**16-1;
        self.set_clock_div(div)
        return int(round(1.0*div/self._base_clock))

    def start(self):
        """ Start transmission. """
        self._iocmd( (0xB0, 0x10, 0x01) )
        return

    def stop(self, instantly=0):
        """ Stop transmission.
        instantly==0: Stop gracefully after the current iteration.
        instantly==1: Stop instantly. (Discards remaining sequence bytes.)"""
        if instantly: self._iocmd( (0xB0, 0x10, 0x00, 0x01) )
        else:         self._iocmd( (0xB0, 0x10, 0x00, 0x00) )
        return

    def reset(self):
        """ Reset transmission and all parameters. """
        self._iocmd( (0xB0, 0xFF) )
        return

    def get_iterations(self):
        """ Query the number of transmission iterations (0:infinite)."""
        r = self._iocmd( (0xC0, 0x01) )   # Query base clock
        return sum( r[i]<<(i*8) for i in range(4) )

    def get_clock_div(self):
        """ Get the bit clock divider. Bitrate = base_clock/div."""
        r = self._iocmd( (0xC0, 0x02) )   # Query base clock
        return sum( r[i]<<(i*8) for i in range(4) )

    def get_mod_clock_div(self):
        """ Get the modulation clock divider. mod_freq = mod_base_clock/mod_div."""
        r = self._iocmd( (0xC0, 0x03) )   # Query base clock
        return sum( r[i]<<(i*8) for i in range(4) )
    
    def get_base_clock(self):
        """ Query the devices base bit clock frequency in Hz. """
        r = self._iocmd( (0xC0, 0x05) )   # Query base clock
        return sum( r[i]<<(i*8) for i in range(4) )

    def get_mod_base_clock(self):
        """ Query the devices modulation base clock frequency in Hz. """
        r = self._iocmd( (0xC0, 0x06) )   # Query base clock
        return sum( r[i]<<(i*8) for i in range(4) )
        
    def get_busy(self):
        """ Get the busy state. True if transmission is in progress. """      
        r = self._iocmd( (0xC0, 0x70) )   # Query tx_busy
        return bool( r[0] )

    def get_sequence_length(self):
        """ Get the size of the stored sequence in bytes """
        r = self._iocmd( (0xC0, 0x71) )  # Query seq_length
        return int( r[0]+r[1]*256 )
    
#----------------------------------------------------------------------------

    class target:
        """ A collection of target devices addressable by the mbug_2151.
        The target device must be created from an mbug_2151 instance, which
        will be used as controller."""
        
        _dev = None     # The controller device
        
        # Target device factories
        def AB440S(self, addr=None):
            return mbug_2151_target.AB440S(dev=self._dev, addr=addr)
        def HE302EU(self, addr=None):
            return mbug_2151_target.HE302EU(dev=self._dev, addr=addr)
        def DMV7008(self, addr=None):
            return mbug_2151_target.DMV7008(dev=self._dev, addr=addr)
        def GT7008(self, addr=None):
            return mbug_2151_target.GT7008(dev=self._dev, addr=addr)
        def FIF4280(self, addr=None):
            return mbug_2151_target.FIF4280(dev=self._dev, addr=addr)
        def IKT201(self, addr=None):
            return mbug_2151_target.IKT201(dev=self._dev, addr=addr)
        def RS200(self, addr=None):
            return mbug_2151_target.RS200(dev=self._dev, addr=addr)
        
    target = target()   # Make this an instance 


#==========================================================================

def mbug_2151_test():
    # Print device list
    devs = list_devices(2151)
    print "Attached MBUG-2151 devices:"
    print devs if devs!=[] else 'None'

    # Open devices
    for ser in devs:
        try:
            print "Device", ser
            dev = mbug_2151(ser)
            sw = dev.target.AB440S()
            sw['A'] = 1
            _time.sleep(1)
            sw['A'] = 0
        finally:
            try: dev.close()
            except: pass

#---------------------------------------------------------------------------
if __name__ == "__main__":
    pass
    mbug_2151_test()
    
#---------------------------------------------------------------------------


