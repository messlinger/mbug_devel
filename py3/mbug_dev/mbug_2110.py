
from .mbug_base import *

#======================================================================
class mbug_2110(mbug):
    """ Represents an MBUG-2110 GPIO device """
    
    _type = 2110

    def __init__(self, serial=None):
        mbug.__init__(self, self._type, serial)

    def gpio_get(self, packed=False):
        """ Get the current gpio input state. 
            packed=True returns bits packed as integer. """    
        self._write( (0xE1, 0x00) )
        d = self._read()
        raw = d[0]+(d[1]<<8)
        if packed:
            return raw
        else:
            return [(raw>>i)&1 for i in range(12)]
       
    def gpio_set(self, bits):
        """ Set the gpio output state. 
            Bits can be passed as list or integer."""
        try: raw = sum([ bool(bit)<<i for (i,bit) in enumerate(bits) ])
        except TypeError: raw = int(bits)
        self._write( (0xE0, 0x00, raw&0xFF, (raw>>8)&0xFF) )
        return
    
    def gpio_tris(self, bits):
        """ Set the gpio tristate state (1:Input 0:Output). 
            Bits can be passed as list or integer."""
        try: raw = sum([ bool(bit)<<i for (i,bit) in enumerate(bits) ])
        except TypeError: raw = int(bits)    # Might be a packed integer
        self._write( (0xE2, 0x00, raw&0xFF, (raw>>8)&0xFF) )
        return

    def counter_start(self, interval):
        """Start the counter. Integration interval in seconds (1us resolution).
        Pass interval 'inf' or 0 for infinite gate time with manual stop."""
        if interval=='inf' or interval==float('inf'): interval=0
        interval = int(interval*1000000)
        if interval > 0xFFFFFFFF: raise ValueError('Maximum interval exceeded ')
        self._write( (0xE6, 0x01, interval&0xFF, (interval>>8)&0xFF,
                      (interval>>16)&0xFF, (interval>>24)&0xFF) )
        return

    def counter_stop(self):
        """ Stop the counter immdiately. Can be read afterwards."""
        self._write( (0xE6, 0x00 ) )
        return

    def counter_read(self):
        """Read the counter value. Blocks until the integration interval exceeded."""
        self._write( (0xE6, 0x02 ) )
        raw = self._read()
        cnt = raw[0] + (raw[1]<<8) + (raw[2]<<16) + (raw[3]<<24)
        return cnt

    def counter_is_ready(self):
        """Query if the counter interval exceeded."""
        self._write( (0xE6, 0x03 ) )
        raw = self._read()
        return bool(raw[0])

    def adc_enable(self, chan):
        """ Enable the analog input channels (1:enable, 0:disable). 
        ADC channels 0..7 are multiplexed with GPIO channels 3..10.
        Chan can be passed as list or integer. """
        try: raw = sum([ bool(bit)<<i for (i,bit) in enumerate(chan) ])
        except TypeError: raw = int(chan)
        self._write( (0xE8, 0x00, raw&0xFF, (raw>>8)&0xFF) )
        return

    def adc_read(self, fmt=int):
        """ Read analog inputs. ADC resolution is 10 bit.
        By default, values are returned as list of integers in range 0..1023 .
        Setting fmt=float will return values as floats in range 0..1 """
        self._write( (0xE9, 0x00) )
        raw = self._read()
        dat = [ raw[ch*2] + (raw[ch*2+1]<<8) for ch in range(8) ]
        if fmt==float: dat = [ 1.0*d/1024 for d in dat]
        return dat

    def pwm_enable(self, channels=1):
        """ Enable the pwm output on GPIO 0. """
        try: chb = sum([ bool(bit)<<i for (i,bit) in enumerate(channels) ]) # try to iterate
        except TypeError: chb = int(channels)
        self._write( (0xE4, 0x00, chb&0xff, (chb>>8)&0xff ) )
        return

    def pwm_set(self, duty):
        """ Set the pwm duty cycle. Duty cycle resolution is 10 bit.
        Duty cycle can be passed as integer in range 0..1023 or
        as float in range 0..1.
        For upward compatibilty, duty can also passed as list. """
        try: duty0 = duty[0] # try to iterate
        except TypeError: duty0 = duty
        if (isinstance(duty0, float) and duty0<1.0):
            duty0 = duty0*1024
        duty0 = int(duty0)
        if duty0<0: duty0 = 0
        if duty0>1023: duty0 = 1023
        self._write( (0xE5, 0x00, duty0&0xff, (duty0>>8)&0xff ) )
        return

    def uart_config(self, baudrate=19200, parity=0):
        """Configure and enable the serial port. This will cancel any pending
        serial I/O and enable transmit and receive. The allowed baudrate range
        is 50 .. 256k. Parity can be set as integer or string: 'none' (0),
        'set' (1), 'clear' (2), 'odd' (3), 'even' (4)."""
        baudrate = int(baudrate)
        br = [(baudrate>>(i*8))&0xff for i in (0,1,2,3)]
        if isinstance(parity,str): parity = parity.upper()
        try: p = {0:0, 'NONE':0, 1:1, 'SET':1, 2:2, 'CLEAR':2,
                  3:3, 'ODD':3, 4:4, 'EVEN':4}[parity]
        except KeyError: raise KeyError('Invalid Parity')
        self._write( [0xEC, 0x02] + br + [p] )
        return

    def uart_disable(self):
        """Disable serial port. This will cancel any active transfer. Data can
        still be written to the transmit queue and read from the receive queue,
        however."""

    def uart_status(self):
        """Query uart status, transmit queue size, receive queue size
        status<0>: TX overflow
        status<1>: RC overflow
        status<2>: RC framing error
        status<3>: RC parity error
        In case of an error, reset uart by a call to uart_config()"""
        self._write( (0xEC, 0x70) )
        ret = self._read()
        status = ret[0]
        txq = ret[2]+(ret[3]<<8)
        rxq = ret[4]+(ret[5]<<8)
        return status, txq, rxq

    def uart_send(self, data):
        """ Send via the serial port. The transmit queue can hold a maximum number
        of 64 bytes bytes."""
        data = bytearray(data)
        maxlen = self._size_out-2    
        while(len(data)>0):
            d = data[:maxlen]
            self._write( bytearray([0xEA,len(d)] ) + d)
            s = self._read()[0]
            if s: raise RuntimeError( 'mbug_2110 uart error: '+bin(s)[2:] )
            data = data[maxlen:]
        return
    
    def uart_recv(self, n=0):
        """ Receive data from the serial port, maximum n bytes. Pass n=0 to read the
        complete receive queue (which can hold a maximum of 64 bytes)."""
        n = int(n)
        if n<0: return bytearray("")
        data = bytearray('')
        while(1):
            if n==0: l = self._size_in-2
            else:
                if len(data)>=n: break
                l = min(self._size_in-2, n-len(data))
            self._write( (0xEB, l) )
            ret = self._read()
            s,r = ret[0], ret[1]
            if s: raise RuntimeError( 'mbug_2110 uart error: '+bin(s)[2:] )
            if r==0: break
            data += ret[2:2+r]
        return data
        

#==========================================================================

def mbug_2110_test():
    # Print device list
    devs = list_devices(2110)
    print("Attached MBUG-2110 devices:")
    print(devs if devs!=[] else 'None')

    # Open devices
    for ser in devs:
        try:
            print("Device", ser)
            d = mbug_2110(ser)
            d.gpio_tris(0x000)
            d.gpio_set(0xfff)
            d.gpio_set(0x000)
            d.pwm_enable(1)
            d.pwm_set(0.2)
            d.uart_config(19200)
            d.uart_send("12345")
            print(d.uart_recv())
        finally:
            try: dev.close()
            except: pass

#---------------------------------------------------------------------------
if __name__ == "__main__":
    pass
    mbug_2110_test()
    