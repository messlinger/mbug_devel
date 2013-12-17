
from mbug_base import *

#==========================================================================
class mbug_2165(mbug):
    """ Represents an MBUG-2165 IR transmitter"""
    
    _type = 2165

    def __init__(self, serial=None):
        mbug.__init__(self, self._type, serial)
        self._base_clock = self.get_base_clock()
        self._mod_base_clock = self.get_mod_base_clock()
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
        
    def set_sequence_times(self, data, unit='ms'):
        """Set a transmission sequence in timed mode. The sequence is interpreted
        as consecutive on/off intervalls (starting with on). The unit is determined
        by the unit parameter, either numerically (in seconds) or as 's' (seconds),
        'ms' (milliseconds), us (microseconds). Default unit is 'ms'.
        The resolution of the sequence times (determined by the time base tau and the
        word length (8 or 16 bits)) is chosen automatically.
        If you want more control over the resolution, use the more specific transmis-
        sion functions set_sequence_8bit() or set_sequence_16bit() together with
        set_tau() or set_clock_div()."""
        if isinstance(unit, int) or isinstance(unit,float): pass
        else:
            try: unit = {'us':1e-6, 'ms':1e-3, 's':1 }[unit]
            except KeyError: KeyError('Invalid unit: '+str(unit))
        for i,d in enumerate(data):
            data[i] *= unit
            if data[i]<40e-6: data[i]=40e-6
        nitems = len(data)
        if nitems>255: raise ValueError('Maximum sequence length is 255.')
        bits = 8 if nitems>127 else 16
        tau = max(data)/2**bits
        div = int(tau*self._base_clock + 1) # NOTE: Take ceiling! (Else data items may overflow.)
        if div >= 2**16:
            div = 2**16-1
            "### Warning: Some sequence items are too long and will be clipped. ###"
        tau = 1.*div/self._base_clock
        if min(data)/tau<10:
            print "### Warning: Some sequence items are too short and will have a low resolution. ###"
        for i,d in enumerate(data):
            data[i] = int(data[i]/tau + 0.5)
        self.set_tau(tau)
        if bits==8: self.set_sequence_times_8bit(data)
        else: self.set_sequence_times_16bit(data)
        return

    def set_sequence_times_8bit(self, data):
        """Set a transmission sequence in timed 8 bit mode. The sequence is
        interpreted as consecutive on/off intervalls (starting with on) in units
        of the timebase tau (set via set_tau() or set_clock_div()).
        The size of a sequence element is limited to 8 bit (<=255). The sequence
        length is limited to a mximum of 255 elements."""
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

    def set_sequence_times_16bit(self, data):
        """Set a transmission sequence in timed 16 bit mode. The sequence is
        interpreted as consecutive on/off intervalls (starting with on) in units
        of the timebase tau (set via set_tau() or set_clock_div()).
        The size of a sequence element is limited to 16 bit (<=65535). The sequence
        length is limited to a mximum of 255 elements."""
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
    
    def set_iterations(self, niter):
        """ Set the number of transmission iterations.
        0: infinite, Maximum: 2**16-1 at the moment. """
        if niter=='inf': niter=0
        if niter<0 or niter>=1<<16:
            raise ValueError
        self._iocmd( (0xB0, 0x01, niter&0xFF, (niter>>8)&0xFF, 0x00, 0x00 ) )
        return

    def set_clock_div(self, div):
        """ Set the clock divider. Bitrate = base_clock/div, tau = base_time*div
        Minimum div in bitstream mode is 10, maximum div is 2**16-1. """
        div = int(div)
        if div<=0: div=1
        if div>=1<<16: div=(1<<16)-1
        self._iocmd( (0xB0, 0x02, div&0xFF, (div>>8)&0xFF, 0x00, 0x00 ) )
        return

    def set_bitrate(self, freq):
        """ Set the bit rate (via clock div)."""
        if freq<=0: raise ValueError('Invalid bitrate.')
        div = round(self._base_clock/freq)
        self.set_clock_div(div)

    def set_timebase(self, tau):
        """ Set the timebase (via clock div)."""
        if tau<=0: raise ValueError('Invalid timebase')
        div = round(self._base_clock*tau)
        self.set_clock_div(div)
        
    def set_mod_clock_div(self, div):
        """ Set the modulation clock divider. freq = mod_base_clock/div.
        Minimum div is 16, maximum div is 4096 at the moment.
        Resolution decreases for div>256.
        Set div=0 to turn off modulation."""
        div = int(div)
        if div!=0 and (div<=0 or div>4096):
            raise ValueError('Invalid clock divider.')
        self._iocmd( (0xB0, 0x03, div&0xFF, (div>>8)&0xFF, 0x00, 0x00 ) )
        return

    def set_mod_freq(self, freq):
        """ Set the modulation frequency (via modulation clock div).
        Set freq=0 to turn off modulation."""
        if freq==0:
            div = 0
        else:
            div = round(self._mod_base_clock/freq)
            if div<16: div=16
            if div>4096: div=4096
            rfreq = self._mod_base_clock/div
            rd = 1.*rfreq/freq-1.
            if abs(rd)>0.01:
                print "Warning. Set modulation frequency to %.1e."%(rfreq)
        self.set_mod_clock_div(div)

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
        def Nikon_L3(self):
            return Nikon_L3(dev=self._dev)
        def Pentax(self):
            return Pentax(dev=self._dev)
        def Canon_RC1(self):
            return Canon_RC1(dev=self._dev)
        def Canon_WLD5000(self):
            return Canon_WLD5000(dev=self._dev)
        
    target = target()   # Make this an instance 


#==========================================================================

import time as _time

#==========================================================================

class mbug_2165_target(object):
    _dev = None          # Controller device
    _bitrate = None       # Default parameters
    _mod_freq = None
    _iterations = None
    _timeout = None

    def __init__(self, dev):
        if hasattr(dev,'_type') and dev._type==2165:
            self._dev = dev
        else:
            raise Exception('mbug_2165_target.init(): Need an mbug_2165 instance as control device.')

    def _wait_busy(self, force=0):
        # Wait for device to become ready
        if self._dev.get_busy():
            if not force:
                tout = _time.time() + self._timeout
                while (self._dev.get_busy()):
                    _time.sleep(0.1)
                    if _time.time()>tout:
                        break
            self._dev.disable()

    def _send(self, sequence, force=0):
        # Wait for device to become ready
        self._wait_busy(force)
        # Resend parameters since the device may have been used for another target in the meantime
        self._dev.set_bitrate(self._bitrate)
        self._dev.set_iterations(self._iterations)
        self._dev.set_mod_freq(self._mod_freq)
        self._dev.set_sequence_bitstream(sequence)
        # Engage!
        self._dev.start()
        
#---------------------------------------------------------------------------

class Nikon_L3(mbug_2165_target):
    """Represents a Nikon ML-L3 compatible camera."""

    # Transmission parameters
    _bitrate = 2507.837  # -> tau=1/(4e6/1595)=0.39875 ms
    _iterations = 2
    _mod_freq = 38400
    _timeout = 1
    _seq_trigger = [0b00011111, 0, 0, 0, 0, 0, 0, 0, 0, 0b00001000, 0b00000001, 0b00000100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

    def __init__(self, dev):
        mbug_2165_target.__init__(self, dev)

    def trigger(self, force=0):
        self._send( self._seq_trigger, force )

#---------------------------------------------------------------------------

class Pentax(mbug_2165_target):
    """Represents a Pentax compatible camera."""

    # Transmission parameters
    _bitrate = 1e3  # -> tau=1 ms
    _iterations = 1
    _mod_freq = 38e3
    _timeout = 1
    _seq_trigger = [0b11111111, 0b00011111, 0b01010101, 0b00010101]

    def __init__(self, dev):
        mbug_2165_target.__init__(self, dev)

    def trigger(self, force=0):
        self._send( self._seq_trigger, force )

#---------------------------------------------------------------------------

class Canon_RC1(mbug_2165_target):
    """Represents a Canon RC-1 compatible camera."""

    # Transmission parameters
    _bitrate = 8333  # -> tau=0.12 ms
    _iterations = 1
    _mod_freq = 32.76e3
    _timeout = 1
    _seq_trigger = [0b00001111, 0, 0, 0, 0, 0, 0, 0 ]*2
    _seq_trigger_3s = [0b00001111, 0, 0, 0, 0, 0 ]*2
    
    def __init__(self, dev):
        mbug_2165_target.__init__(self, dev)

    def trigger(self, force=0):
        self._send( self._seq_trigger, force )

    def trigger_3s(self, force=0):
        self._send( self._seq_trigger_3s, force )

#---------------------------------------------------------------------------

class Canon_WLD5000(mbug_2165_target):
    """Canon Video camera remote control WL-D5000"""
    
    # similar to NEC protocol,
    #  - no repeat shortcut (message is repeated in full length)
    #  - 2nd addr field inversion is not obeyed
    
    _bitrate = 7142  # -> tau=0.14 ms, 1.12 ms per byte
    _iterations = 3
    _mod_freq = 38e3
    _timeout = 1
    _1_, _0_ = [0x0f,0], [0x0f]
    _seq_trigger = [0xff]*8 + [0]*4 \
                   + _1_+_0_+_1_+_0_+_0_+_0_+_0_+_1_ + _0_+_1_+_1_+_0_+_1_+_1_+_1_+_0_ \
                   + _0_+_0_+_0_+_0_+_1_+_1_+_0_+_0_ + _1_+_1_+_1_+_1_+_0_+_0_+_1_+_1_ \
                   + _0_ + [0]*36

## Timed mode:
##    _seq_trigger = [9.00, 4.45] + [ (0.59,1.65)[b] for b in \
##        (1,0,1,0,0,0,0,1,  0,1,1,0,1,1,1,0,  0,0,0,0,1,1,0,0,  1,1,1,1,0,0,1,1) ] 
    
    def __init__(self, dev):
        mbug_2165_target.__init__(self, dev)

    def trigger(self, force=0):
        self._send( self._seq_trigger, force )

#==========================================================================

def mbug_2165_test():
    # Print device list
    devs = list_devices(2165)
    print "Attached MBUG-2165 devices:"
    print devs if devs!=[] else 'None'

    # Open devices
    for ser in devs:
        try:
            print "Device", ser
            dev = mbug_2165(ser)
            ##tar = mbug_2165.target.Canon_WLD5000()
            ##tar.trigger()
            dev.set_sequence_times([0.1,0.1,0.2,0.2,0.5,0.5,1,1,2,2,5,5,10,10,20,20,50,50,100,100])
            ##dev.set_iterations(3)
            ##dev.set_clock_div(4)
            ##dev.set_sequence_times_16bit([100,100,200,200,500,500,1000,1000,10000,10000])
            ##dev.set_sequence_times([100,100,200,200,500,500,1000,1000,10000,10000],'us')
            ##dev.set_sequence_times([0.1,0.1,0.2,0.2,0.5,0.5,2,2,5,5,10,10,20,20,50,50,100,100],'ms')
            ##dev.set_sequence_times([0.1,0.1,0.2,0.2,0.5,0.5,2,2,5,5,10,10,20,20],'ms')
            dev.start()
        finally:
            try: dev.close()
            except: pass

#---------------------------------------------------------------------------
if __name__ == "__main__":
    pass
    mbug_2165_test()
        
#---------------------------------------------------------------------------
