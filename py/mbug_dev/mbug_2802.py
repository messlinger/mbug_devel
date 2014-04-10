import sys
from .mbug_base import *

#==========================================================================
class mbug_2802(mbug):
    """ Represents an MBUG-2802 thermometer device """

    _type = 2812

    def __init__(self, serial=None):
        mbug.__init__(self, self._type, serial)

    def adc_io(self, to_adc, nread=0):
        """Direct arbitrary I/O with AD-Converter. """
        try:
            nwrite = len(to_adc)
            to_adc = [ d&0xFF for d in to_adc[:nwrite] ]
        except TypeError:
            to_adc = ( int(to_adc)&0xFF, )
            nwrite = 1
        if nwrite>18: nwrite = 18
        if nread>16: nread=16
        outp = bytearray( (0xC0, 0xFF, nwrite, nread) ) \
             + bytearray( to_adc[:nwrite] )
        self._write( outp )
        inp = self._read()
        if inp[0:4] != outp[0:4]:
            raise Exception('mbug_2812.adc_io(): Communication error. Received:'+repr(inp))
        return inp[4:4+nread]

    def adc_rreg(self, addr=None, nregs=1):
        """ADC command RREG: Read single/multiple register(s).
        addr: First register to read (0..15).
        nregs: Number of registers to read.
        Default: Read all registers.
        Returns single register content or list of registers for nregs>1."""
        if addr==None: addr, nregs = 0, 15
        addr = int(addr)
        if addr<0 or addr>15: raise Exception("Invalid address.")
        if nregs>15: raise Exception("Invalid length.")
        r = self.adc_io( (RREG|addr,nregs-1), nregs)
        if nregs>1:  return r[:nregs]
        else: return r[0]

    def adc_wreg(self, addr, data):
        """ADC command WREG: Write single/multiple register(s).
        addr: First register to write to (0..15).
        data: Data byte to write to the register, or list-like of data bytes
              to write to several registers starting with addr (len=0..15).
        """
        addr = int(addr)
        if addr<0 or addr>15: raise Exception("Invalid address.")
        try: # Lets assume data is a list of bytes
            n = len(data)
            if n<1 or n>15: raise Exception("Invalid length.")
            self.adc_io( [0x40|addr, n-1] + [d&0xFF for d in data] )
        except TypeError: # Doesn't seem to be list-like. treat as int
            data = int(data)&0xFF
            self.adc_io( (WREG|addr, 0, data) )

    def adc_rdata(self):
        """ADC command RDATA: Read last conversion result."""
        r = self.adc_io( (RDATA), 3 )
        data = (r[0]<<16) + (r[1]<<8) + r[2]
        if r[0]>0x7F: data -= 0x1000000
        return data

    def adc_sync(self):
        """ADC command SYNC: Start new conversion now."""
        self.adc_io( SYNC )
            
#==========================================================================
# ADS1248 SPI command definitions
NOP  	  = 0xFF	
WAKEUP	  = 0x00
SLEEP	  = 0x02
SYNC	  = 0x04
RESET	  = 0x06
RDATA	  = 0x12
RDATAC	  = 0x14
SDATAC	  = 0x16
RREG	  = 0x20
WREG	  = 0x40
SYSOCAL   = 0x60
SYSGCAL   = 0x61
SELFOCAL  = 0x62

# ADS1248  register addresses
MUX0     = 0x00
VBIAS    = 0x01
MUX1	 = 0x02
SYS0	 = 0x03
OFC0	 = 0x04
OFC1	 = 0x05
OFC2	 = 0x06
FSC0	 = 0x07
FSC1	 = 0x08
FSC2	 = 0x09
IDAC0    = 0x0A
IDAC1    = 0x0B
GPIOCFG  = 0x0C
GPIODIR  = 0x0D
GPIODAT  = 0x0E

#==========================================================================
def mbug_2802_test():
    # Print device list
    devs = list_devices(2812)
    print("Attached MBUG-2812 devices:")
    print(devs if devs!=[] else 'None')

    # Open devices
    for ser in devs:
        try:
            print("Device", ser)
            dev = mbug_2812(ser)
            print("No test implemented for this device")
        finally:
            dev.close()

#--------------------------------------------------------------------------    
#if __name__ == "__main__":
#    mbug_2812_test()

from time import sleep

try: d.close()
except: pass
d = mbug_2802()
sleep(0.25)

#--------------------------------------------------------------------------

def meas(Iex,pga,Rref=100):
    codes_Iex = {0:0b000, 0.05: 0b001, 0.1:0b010, 0.25:0b011, 0.5:0b0100, 0.75:0b101, 1.0:0b110, 1.5:0b111}
    codes_pga = {1:0b000, 2:0b001, 4:0b010, 8:0b011, 16:0b100, 32:0b101, 64:0b110, 128:0b111}
    Iex = codes_Iex.get(Iex, 0)
    pga = codes_pga.get(pga, 0)
    if not Rref in [100,1000]: Rref=100
    
    d.adc_wreg(MUX1, 0b01010000)        # IntOsc, IntRef, IntRef On
    d.adc_wreg(SYS0, 0b00000000 | (pga<<4))        # PGA, 5 SPS

    # Measurement of Ur:
    d.adc_wreg(VBIAS, 0b00000000)       # Bias off

    if Rref==1000:
        d.adc_wreg(GPIOCFG, 0b00000001)     # GOPIO(REFN0)=0 (Ref-100R)
        d.adc_wreg(GPIODIR, 0b11111110)     #
        d.adc_wreg(GPIODAT, 0b00000000)     #
    else: # Rref=100
        d.adc_wreg(GPIOCFG, 0b00000010)     # GOPIO(REFP0)=0 (Ref-1k)
        d.adc_wreg(GPIODIR, 0b11111101)     #
        d.adc_wreg(GPIODAT, 0b00000000)     #


    d.adc_wreg(IDAC0, 0b00000000 | Iex)       # Iex 
    d.adc_wreg(IDAC1, 0b11111000)       # -> to Pin EXT1 (Sensor 1)
    sleep(0.1)

    d.adc_wreg(MUX0, 0b00010011)        # Ain2-Ain3 (Ur)
    sleep(0.25)
    Ur = d.adc_rdata()

    # Measurement of Uref
    if Rref==1000:
        d.adc_wreg(MUX0, 0b00101001)        # Ain5-Ain1 (Uref-1k)
    else:
        d.adc_wreg(MUX0, 0b00100000)        # Ain4-Ain0 (Uref-100R)

    sleep(0.25)
    Uref = d.adc_rdata()

    Or,Oref = 0, 0
    # Measurement of Offset Ur
    ##d.adc_wreg(GPIOCFG, 0b00000000)     # GPIO off
    ##d.adc_wreg(IDAC1, 0b11111111)       # Ix off
    ##d.adc_wreg(VBIAS, 0b00100000)       # Bias to Ain5
    ##sleep(0.1)

    ##d.adc_wreg(MUX0, 0b00010011)        # Ain2-Ain3 (Ur)
    ##sleep(0.25)
    ##Or = d.adc_rdata()

    # Measurement of Offset Uref
    ##d.adc_wreg(MUX0, 0b00100000)        # Ain4-Ain0 (Uref-1k)
    ##sleep(0.25)
    ##Oref = d.adc_rdata()

    return Ur,Or,Uref,Oref

from time import time

##for Iex in [0.25, 0.5, 0.25, 0.75, 0.25, 1.0, 0.25, 1.5, 0.25, 1.5, 0.25, 1.0, 0.25, 0.75, 0.25, 0.5, 0.25]:
for Iex in [0.05, 0.1, 0.05, 0.25, 0.05, 0.5, 0.05, 0.5, 0.05, 0.25, 0.05, 0.1, 0.05]:
    print() 
    for N in range(200):
        Ur,Or,Uref,Oref = meas(Iex, pga=2) 
        sys.stdout.write('%f\t ' % time() )
        sys.stdout.write('%f\t ' % (1e2*(Ur-Or)/(Uref-Oref)) )
        print( Ur, '\t', Or, '\t', Uref, '\t', Oref, '\t', Iex )
        






##print
##print
##print '#'
##print '# Pt100'
##print '# Iex=0.75mA, PGA=16'
##print '#'
##print '# time\tR\tUr\tOr\tUref\tOref'
##print '#-------------------------------------------------------------'
##
##while(1):
##        
##    d.adc_wreg(MUX1, 0b01010000)        # IntOsc, IntRef, IntRef On
##    d.adc_wreg(SYS0, 0b01000000)        # PGA=16, 5 SPS
##
##    # Measurement of Ur:
##    d.adc_wreg(VBIAS, 0b00000000)       # Bias off
##
##    d.adc_wreg(GPIOCFG, 0b00000010)     # GOPIO(REFN0)=0 (Ref-100R)
##    d.adc_wreg(GPIODIR, 0b11111101)     #
##    d.adc_wreg(GPIODAT, 0b00000000)     #
##
##    d.adc_wreg(IDAC0, 0b00000101)       # Iex = 1.0 mA
##    d.adc_wreg(IDAC1, 0b11111000)       # -> to Pin EXT1 (Sensor 1)
##    sleep(0.1)
##
##    d.adc_wreg(MUX0, 0b00010011)        # Ain2-Ain3 (Ur)
##    sleep(0.25)
##    Ur = d.adc_rdata()
##
##    # Measurement of Uref
##
##    d.adc_wreg(MUX0, 0b00100000)        # Ain4-Ain0 (Uref-100R)
##    sleep(0.25)
##    Uref = d.adc_rdata()
##
##    # Measurement of Offset Ur
##    d.adc_wreg(GPIOCFG, 0b00000000)     # GPIO off
##    d.adc_wreg(IDAC1, 0b11111111)       # Ix off
##    d.adc_wreg(VBIAS, 0b00100000)       # Bias to Ain5
##    sleep(0.1)
##
##    d.adc_wreg(MUX0, 0b00010011)        # Ain2-Ain3 (Ur)
##    sleep(0.25)
##    Or = d.adc_rdata()
##
##    # Measurement of Offset Uref
##    d.adc_wreg(MUX0, 0b00100000)        # Ain4-Ain0 (Uref-1k)
##    sleep(0.25)
##    Oref = d.adc_rdata()
##
##    ##    print 'Ur:   ', 2.048*Ur/0x7fffff
##    ##    print 'Or:   ', 2.048*Or/0x7fffff
##    ##    print 'Uref: ', 2.048*Uref/0x7fffff
##    ##    print 'Oref: ', 2.048*Oref/0x7fffff
##    ##
##    ##    print 'Resistance from U/Uref:', 1e2*(Ur-Or)/(Uref-Oref)
##    ##    print 'Resistance from U/Iex: ', 2.048*(Ur-Or)/0x7fffff/0.1e-3/4.
##
##    print time(), '\t',
##    print 1e2*(Ur-Or)/(Uref-Oref), '\t',
##    print Ur, '\t', Or, '\t', Uref, '\t', Oref, '\t'
##
##





##
##from time import time
##
##print
##print
##print '#'
##print '# Vishay 1k'
##print '# Iex=0.1mA, PGA=8'
##print '#'
##print '# time\tR\tUr\tOr\tUref\tOref'
##print '#-------------------------------------------------------------'
##
##while(1):
##    d.adc_wreg(MUX1, 0b01010000)        # IntOsc, IntRef, IntRef On
##    d.adc_wreg(SYS0, 0b00110000)        # PGA=8, 5 SPS
##
##    # Measurement of Ur:
##    d.adc_wreg(VBIAS, 0b00000000)       # Bias off
##
##    d.adc_wreg(GPIOCFG, 0b00000001)     # GOPIO(REFP0)=0 (Ref-1k)
##    d.adc_wreg(GPIODIR, 0b11111110)     #
##    d.adc_wreg(GPIODAT, 0b00000000)     #
##
##    d.adc_wreg(IDAC0, 0b00000010)       # Iex = 0.1mA
##    d.adc_wreg(IDAC1, 0b11111000)       # -> Pin EXT1 (Sensor 1)
##    sleep(0.1)
##
##    d.adc_wreg(MUX0, 0b00010011)        # Ain2-Ain3 (Ur)
##    sleep(0.25)
##    Ur = d.adc_rdata()
##
##    # Measurement of Uref
##
##    d.adc_wreg(MUX0, 0b00101001)        # Ain5-Ain1 (Uref-1k)
##    sleep(0.25)
##    Uref = d.adc_rdata()
##
##    # Measurement of Offset Ur
##    d.adc_wreg(GPIOCFG, 0b00000000)     # GPIO off
##    d.adc_wreg(IDAC1, 0b11111111)       # Ix off
##    d.adc_wreg(VBIAS, 0b00010000)       # Bias to Ain4
##    sleep(0.1)
##
##    d.adc_wreg(MUX0, 0b00010011)        # Ain2-Ain3 (Ur)
##    sleep(0.25)
##    Or = d.adc_rdata()
##
##    # Measurement of Offset Uref
##    d.adc_wreg(MUX0, 0b00101001)        # Ain5-Ain1 (Uref-1k)
##    sleep(0.25)
##    Oref = d.adc_rdata()
##
####    print 'Ur:   ', 2.048*Ur/0x7fffff
####    print 'Or:   ', 2.048*Or/0x7fffff
####    print 'Uref: ', 2.048*Uref/0x7fffff
####    print 'Oref: ', 2.048*Oref/0x7fffff
####
####    print 'Resistance from U/Uref:', 1e3*(Ur-Or)/(Uref-Oref)
####    print 'Resistance from U/Iex: ', 2.048*(Ur-Or)/0x7fffff/0.1e-3/4.
##
##    print time(), '\t',
##    print 1e3*(Ur-Or)/(Uref-Oref), '\t',
##    print Ur, '\t', Or, '\t', Uref, '\t', Oref, '\t'
##    
##
##
##

