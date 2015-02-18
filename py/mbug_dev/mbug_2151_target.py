
import time as _time

#==========================================================================

class mbug_2151_target(object):
    _dev = None          # Controller device
    _timebase = None       # Default parameters     
    _iterations = 3
    _seq_type = 'bitstream'
    _timeout = None

    def __init__(self, dev):
        if hasattr(dev,'_type') and dev._type==2151:
            self._dev = dev
        else: raise Exception('mbug_2151_target.init(): Need an mbug_2151 instance as control device.')

    def _wait_busy(self, force=0):
        # Wait for device to become ready
        if self._dev.get_busy():
            iter = self._dev.get_iterations()
            if force or iter==0: self._dev.stop(instantly=1)
            tout = _time.time() + self._timeout
            while (self._dev.get_busy()):
                _time.sleep(0.01)
                if _time.time()>tout: break

    def _send(self, sequence, force=0):
        self._wait_busy(force)
        self._dev.set_timebase(self._timebase)
        self._dev.set_iterations(self._iterations)
        self._dev.set_sequence(sequence, self._seq_type)
        self._dev.start()

#---------------------------------------------------------------------------

class AB440S(mbug_2151_target):
    """Elro AB440S remote controlled power switches. This device class represents a bunch
    of addressable switches or a single switch (if instantiated with given address)."""

    # Transmission parameters
    _timebase = 320e-6
    _iterations = 10
    _timeout = 2
    _addr = None

    def __init__(self, dev, addr=None):
        mbug_2151_target.__init__(self, dev)
        self.addr = addr

    def _addr_sequence(self, addr, on):
        # Address sequence for AB440S (PT2262 compatible):
        # 10x DIP-switches: 1 2 3 4 5 A B C D E  ON:  floating
        #                   |                    OFF: gnd
        #                   Highest sequence bit
        # Power on:  xxxxxxxxxx0f,
        # Power off: xxxxxxxxxxf0
        # Note: For the original remote control to be usable, only one of the switches
        #       A-E may be on.
        # Specify address:
        #   As integer: Binary representation of all 10 DIP switches, with switch '1'
        #               being the most siginficant bit, 'E' the least significant bit
        #   As string:  String contains exactly the switche names that are ON, e.g.: 
        #               '15C'   -> 0b1000100100
        #               '34AE'  -> 0b0011010001

        if isinstance(addr,str):
            sw = {'1':9, '2':8, '3':7, '4':6, '5':5, 'A':4, 'B':3, 'C':2, 'D':1, 'E':0,}
            addr = sum([2**sw.get(c,0) for c in addr if c in sw])
        seq = bin((2<<9)|addr)[-10:]
        seq = seq.replace('0','f').replace('1','0')        
        seq += '0f' if on else 'f0'

        # Convert PT2262 to byte sequence
        tok = {'0': 0b00010001, '1': 0b01110111, 'f': 0b01110001, 'x': 0b00010111 }
        bytes = bytearray( [tok.get(c,tok['x']) for c in seq] )
        bytes[12:16] = (0x01, 0x00, 0x00, 0x00)
        return bytes

    def switch(self, addr, on, force=0):
        """Turn addressed remote switch on/off.  The address of a power switch target
        can beset via 10 DIP switches at the device: 1 2 3 4 5 A B C D E
        The switch() function accepts the target address as integer or string.
           An integer is interpreted as binary representation of the 10 DIP switches
        with switch 1 being the most siginificant bit, switch E being the least
        siginficant bit.
           A string is interpreted as containing exactely the names of the switches
        that are in position ON, e.g.:
             '15C'   -> 0b1000100100
             '34AE'  -> 0b0011010001
        """
        self.addr = addr
        seq = self._addr_sequence(addr, on)
        self._send( seq, force )

    def __setitem__(self, addr, on):
        """Subscript set operator as shortcut for switch()."""
        self.switch(addr, on)

    def __getitem__(self, addr):
        """Subscript get operator. Returns a switch device instance with the
        correpsonding address."""
        return self.__class__(self._dev, addr)

    def on(self,on=1):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,on)
        
    def off(self):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,0)        

#---------------------------------------------------------------------------

class LUC308554(mbug_2151_target):
    """LUC Art.Nr. 308554 remote controlled power switches. This device class represents a bunch
    of addressable switches or a single switch (if instantiated with given address)."""

    # Transmission parameters
    _timebase = 182e-6
    _iterations = 10
    _timeout = 2
    _addr = None

    def __init__(self, dev, addr=None):
        mbug_2151_target.__init__(self, dev)
        self.addr = addr

    def _addr_sequence(self, (syscode, addr), on):
        # The 7-bit system code is fixed internally via solder pads at a the address
        # pins of the HS-2262 encoder.  For the ease of use, we treat a closed
        # solder pad as 1, an open solder pad as 0 and pin 1 as the leftmost bit.
        # Therfore, the syscode can be read from the solder pads as binary number
        # from left to right.
        # However, since there are only 128 possible systemcodes, finding the code
        # by brute force takes less than a minute, so there is no need to open
        # the housing.
        # The 3 receivers per system are addressed by special codes on the A7,D1-D0
        # bits, that result from the electrical wiring of the buttons.
        #
        syscode = int(syscode)
        if syscode<0 or syscode>127: raise ValueError("Invalid system code")
        addr = int(addr)
        if addr<1 or addr>3: raise ValueError("Invalid switch address")
        seq = bin((1<<7)|syscode)[-7:]
        seq = seq.replace('0','f').replace('1','0')
        seq += {1:'f01', 2:'f10', 3:'100'}[addr]
        seq += '01' if on else '10'

        # Convert PT2262 to byte sequence
        tok = {'0': 0b00010001, '1': 0b01110111, 'f': 0b01110001, 'x': 0b00010111 }
        bytes = bytearray( [tok.get(c,tok['x']) for c in seq] )
        bytes[12:16] = (0x01, 0x00, 0x00, 0x00)
        return bytes

    def switch(self, addr, on, force=0):
        """Turn addressed remote switch on/off. The address of a switch target consists of
        2 numbers: The 7 bit system code and the receiver address (1-3).
        Use with tuple (syscode,addr) as addr, or integer addr only, in which case a
        previously stored syscode is used (default syscode = 0). System codes are hard coded
        into a specific transmitter. If you do not know your syscode, try out all values 0..127,
        which takes less than a minute.
        """
        try: addr = (addr[0],addr[1])
        except: addr = (self.addr[0],addr)
        self.addr = addr
        seq = self._addr_sequence(addr, on)
        self._send( seq, force )
    
    def __setitem__(self, addr, on):
        """Subscript set operator as shortcut for switch().
        Use with tuple (syscode,addr) as addr, or integer addr only, in which case a
        previously stored syscode is used (default syscode = 0)."""
        self.switch(addr, on)

    def __getitem__(self, addr):
        """Subscript get operator. Returns a switch device instance with the
        corresponding address. Use with tuple (syscode,addr) as index, or addr
        only, in which case a previously stored syscode is used (default
        syscode = 0)."""
        return self.__class__(self._dev, addr)

    def on(self,on=1):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,on)
        
    def off(self):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,0)      

#-----------------------------------------------------------------------

class HE302EU(mbug_2151_target):
    """HomeEasy HE302EU remote controlled power switches. This device class represents a bunch
    of addressable switches or a single switch (if instantiated with given address)."""

    # Transmission parameters
    _timebase = 256e-6
    _iterations = 5
    _timeout = 2
    addr = None

    def __init__(self, dev, addr=None):
        mbug_2151_target.__init__(self, dev)
        self.addr = addr

    def _addr_sequence(self, addr, on):
        # Address sequence for HE302EU:
        # 4 Group numbers:  1,2,3,4 (typed roman on the remote control)
        # 4 Switch numbers: 1,2,3,4
        # The manual defines a combination of group and switch number as
        # a 'function':
        # 1-1, 1-2, 1-3, 1-4 -> Functions 1,2,3,4
        # 2-3, 2-3, 2-3, 2-4 -> Functions 5,6,7,8
        # 3-3, 3-3, 3-3, 3-4 -> Functions 9,10,11,12
        # 4-3, 4-3, 4-3, 4-4 -> Functions 13,14,15,16
        # There is also a a group on/off switch, which, despite of the
        # name, is independent of the group setting.  Note, that a switch
        # in this system can be assigned multiple addresses, so that it
        # can be controlled by more than one remote control sender unit.
        #
        # Pass address as string containing group and switch number,
        # eg. '11', '24', or integer representing the function number.
        # The group number can also be passed as roman number, eg.
        # 'I1', 'IV4'
        # An address of None, 0, or 'G' will be interpreted as group address.
        #
        # The sequence is encodes using 4 different sub-sequences, distinguished
        # by 1 to 4 short pulses (256 us high/low) followed by a longer gap (5*256 us):
        # 1: 100000
        # 2: 10100000
        # 3: 1010100000
        # 4: 101010100000
        # Note, that the sub-sequences are grouped together, so that 4 consecutive
        # ones add up to 7 pulses. 
        # The complete sequence consists of a lengthy preamble followed by
        # a code for on/off, followed by the receiver address.  I have not yet
        # found an analytical relation between receiver address and sequence.

        seq_head = '11 4111 3211 1213 1213 3112 1411'   # preamble
        seq_g_on = '1321 4111'; seq_g_off = '1411 4111' # group on/off
        seq_on  = '2113'; seq_off = '2122'              # single on/off
        seq_addr = { 1:'4111',  2:'2113',  3:'3121',  4:'3112', # single addresses
                     5:'2311',  6:'2221',  7:'2212',  8:'2131',
                     9:'2122', 10:'2113', 11:'1411', 12:'1321',
                    13:'1312', 14:'1231', 15:'1222', 16:'1213',}
       
        seq = seq_head
        if isinstance(addr, str):
            lett = {'I':'1', 'II':'2', 'III':'3', 'IV':'4',
                    'A':'1',  'B':'2',   'C':'3',  'D':'4', 'G':'0'}
            for r,n in lett.items():
                addr = addr.replace(r,n)
            if addr[0:2].isalnum():
                addr = int(addr[0])*int(addr[1])
            else: raise Exception('Invalid address.')

        if addr==None: addr = 0
        addr = int(addr)
        if addr not in seq_addr: addr = 0   # use group
        if addr==0:
            seq += seq_g_on if on else seq_g_off
        else:
            seq += seq_on if on else seq_off
            seq += seq_addr[addr]

        for i in [1,2,3,4]:
            seq = seq.replace( str(i),'10'*i + '0'*4 )

        bseq = [0]; i = 0
        for s in seq:
            if   s=='1': bseq[-1] += 1<<(i%8)
            elif s=='0': pass
            else: continue
            i += 1
            if (i%8 == 0): bseq.append(0)
        return bseq

    def switch(self, addr, on, force=0):
        """Turn addressed remote switch on/off.  Pass the address as integer
        representing the function number (1-16) or as string containing the
        group and switch number, eg. '11', '43'. The group number can also be
        passed as roman number, eg. 'I1', 'IV3'. An addr=None, 0 or 'G' will be
        interprested as group switch command.
        """
        self.addr = addr
        seq = self._addr_sequence(addr, on)
        self._send( seq, force )

    def __setitem__(self, addr, on):
        """Subscript set operator as shortcut for switch()."""
        self.switch(addr, on)

    def __getitem__(self, addr):
        """Subscript get operator. Returns a switch device instance with the
        correpsonding address."""
        return self.__class__(self._dev, addr)

    def on(self,on=1):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,on)
        
    def off(self):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,0)        
      
        
     
#---------------------------------------------------------------------------

class DMV7008(mbug_2151_target):
    """Dario DMV-7008
    Globaltronics GT-7008
    FiF model 4280
    remote controlled power switches. This device class represents a bunch
    of addressable switches or a single switch (if instantiated with given address).
    Dimmer function not implemented yet."""

    # Transmission parameters
    _timebase = 120e-6  # 1.92ms / 16
    _iterations = 4
    _timeout = 2
    syscode, addr = None, None

    def __init__(self, dev, addr=None):
        mbug_2151_target.__init__(self, dev)
        try: self.syscode, self.addr = addr[0], addr[1]
        except: self.syscode, self.addr = 0, None

    def _addr_sequence(self, (syscode, addr), cmd):
        # Addresses consist of a 12 bit system code + 8 bits addr/cmd code.
        # The system code is randlomly chosen for a given remote control,
        # and can be learned by the receivers. Per system, there are 4 known
        # receiver addresses (1..4) + a master address that controls all
        # receivers with matching system code. For master address, pass
        # addr=0, 'master' or 'all'.
        # There are 4 known commands: "on, off, hell, dimmen". The latter two
        # are only valid for dimmable receivers: "hell" increases the current,
        # "dimmen" decreases the current.
        # The commands are combined together with the receiver addresses to
        # an 8 bit code (that looks like arbitrarily chosen to me). 
        
        syscode = int(syscode)&0xfff
        if addr in ('m','M','master','all'): addr=0
        if not addr in (0,1,2,3,4): raise ValueError('Invalid address.')
        if cmd in ('hell','dimmen'): cmd={'hell':2, 'dimmen':3}[cmd]
        if not cmd in (0,1,2,3): raise ValueError('Invalid command.')

        bits_addr_cmd = { ## 0:off, 1:on, 2:hell, 3:dimmen
            0: {0: 0b11100001, 1: 0b11110000, 2: 0b11101011, 3: 0b11111010},  # master (all receivers) 
            1: {0: 0b00000000, 1: 0b00010001, 2: 0b00001010, 3: 0b00011011},  # channel 1
            2: {0: 0b10000010, 1: 0b10010011, 2: 0b10001000, 3: 0b10011001},  #         2
            3: {0: 0b01000001, 1: 0b01010000, 2: 0b01001011, 3: 0b01011010},  #         3
            4: {0: 0b11000011, 1: 0b11010010, 2: 0b11001001, 3: 0b11011000},  #         4
        }

        bits =  (1<<20) + (syscode<<8) + bits_addr_cmd[addr][cmd]  # start bit, syscode, chan/cmd

        pulse = [[0b11111111, 0b11100000], [0b11111000, 0b00000000]] # Approximate 1/3 pulse width

        seq = []
        for i in range(21):
            seq += pulse[(bits>>i)&1]
        seq.reverse()
        seq += [0]*(118-len(seq))   # Repetition rate: 112.8 ms = 59 pulse periods
        
        return seq
        

    def switch(self, addr, on, force=0):
        """Turn addressed remote switch on/off. The address of a switch target consists of
        2 numbers: The 12 bit system code and the receiver address (0-4, 0=all).
        Use with tuple (syscode,addr) as addr, or integer addr only, in which case a
        previously stored syscode is used (default syscode = 0)."""
        try:
            syscode, addr = addr[0], addr[1]
            self.syscode, self.addr = syscode, addr
        except:
            syscode = self.syscode
            self.addr = addr
        seq = self._addr_sequence((syscode,addr), on)
        self._send( seq, force )
        
    def dim(self, addr, level, force=0):
        """Dim addressed remote switch. The address of a switch target consists of
        2 numbers: The 12 bit system code and the receiver address (0-4, 0=all).
        Use with tuple (syscode,addr) as addr, or integer addr only, in which case a
        previously stored syscode is used (default syscode = 0).
        if abs=0 (default), level gives the dim steps relative (positive or negative)
        to the currently set level. For the DMV-7008, the maximum dim level is 50.
        For the FiF 4280, the maximum dim level is about 150. Note, that dimming is
        not completely reliable, since the receivers sometimes drop single commands."""
        try: addr = (addr[0],addr[1])
        except: addr = (self.addr[0],addr)
        level = int(level)
        cmd = 2 if level>0 else 3
        steps = abs(level)

        # Wait for device to become ready
        if not force:
            self._wait_busy(self._timeout)
            
        # Resend transmission parameters, since a different target
        # may have been used in the meantime
        self._dev.set_timebase(self._timebase)
        self._dev.set_iterations(1)
        seq = self._addr_sequence(addr, cmd)        
        self._dev.set_sequence(seq, self._seq_type)
        for i in range(steps):
            self._wait_busy()
            _time.sleep(0.02)
            self._dev.start()        

    def __setitem__(self, addr, on):
        """Subscript set operator as shortcut for switch().
        Use with tuple (syscode,addr) as addr, or integer addr only, in which case a
        previously stored syscode is used (default syscode = 0)."""
        self.switch(addr, on)

    def __getitem__(self, addr):
        """Subscript get operator. Returns a switch device instance with the
        corresponding address. Use with tuple (syscode,addr) as index, or addr
        only, in which case a previously stored syscode is used (default
        syscode = 0)."""
        return self.__class__(self._dev, addr)

    def on(self,on=1):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,on)
        
    def off(self):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,0)        
        
#-----------------------------------------------------------------------
# Hardware identical to DMV7008:
GT7008 = DMV7008
FIF4280 = DMV7008

#---------------------------------------------------------------------------

class IKT201(mbug_2151_target):
    """IKT201 Remote power system (UPM/Ikea). This device class represents a
    bunch of addressable switches/dimmers or a single switch (if instantiated
    with given address)."""

    # Transmission parameters
    _timebase = 214e-6
    _iterations = 2
    _timeout = 2
    addr = None

    def __init__(self, dev, addr=None):
        mbug_2151_target.__init__(self, dev)
        self.addr = addr

    def _sequence(self, (syscode, channels), on=None, level=None, rel=None, gradual=0):
        
        syscode = int(syscode-1)&0x0f
        try:
            channels = [int(i%10) for i in channels] # to iterate
        except:
            try: channels = [int(channels%10)]
            except: raise TypeError("Invalid channel list")
        if on==None and level==None and rel==None:
            raise ValueError("Invalid command")
        if level!=None:
            level = int(level)
            if level<0 or level>9: raise ValueError("Invalid level")
            if level==0: level = 0x0A
            cmd = (level&0x0f) + 0x10
        if on!=None:
            if on: cmd = 0x00 + 0x10
            else: cmd = 0x0A + 0x10
        if rel != None:
            raise ValueError("Relative level not implemented")
        gradual = bool(gradual)
        
        d = [1,1,1,0] ## header
        d += [(syscode>>i)&1 for i in (0,1,2,3)] ## system code
        d += [int(i in channels) for i in range(10)] ## channels
        d += [1^d[-10]^d[-8]^d[-6]^d[-4]^d[-2], 1^d[-9]^d[-7]^d[-5]^d[-3]^d[-1]]  ## checksum(channels)
        d += [(cmd>>i)&1 for i in (0,1,2,3,4)]  ## command
        d += [int(gradual)]  
        d += [1^d[-6]^d[-4]^d[-2], 1^d[-5]^d[-3]^d[-1]]  ## checksum(command)

        # biphase-mark-encoding. 1 sequence byte per encoded bit
        seq = [0x0F + 0xF0*(d[0]^1)]
        for b in d[1:]:
            seq += [0x0F*((seq[-1]>>7)^1) + 0xF0*((seq[-1]>>7)^1^b)]
        seq += [0]*39  ## 66 ms gap
        return seq
        

    def switch(self, addr, on, force=0):
        """Turn addressed remote switches on/off. The address of a switch target consists of
        2 numbers: The system code (1-16) and a list of addressed receivers (0-10).
        Use with tuple (syscode,addrlist) as addr, or addrlist only, in which case a
        previously stored syscode is used (default syscode = 0)."""
        try: addr = (addr[0],addr[1])
        except: addr = (self.addr[0],addr)
        self.addr = addr
        seq = self._sequence(addr, on)
        self._send( seq, force )
        
    def set_level(self, addr, level, force=0):
        """Set addressed remote switches to absolute level (0-9), 0=off / 9=max.
        The address of a switch target consists of
        2 numbers: The system code (1-16) and a list of addressed receivers (0-10).
        Use with tuple (syscode,addrlist) as addr, or addrlist only, in which case a
        previously stored syscode is used (default syscode = 0)."""
        try: addr = (addr[0],addr[1])
        except: addr = (self.addr[0],addr)
        self.addr = addr
        seq = self._sequence(addr, level=level)
        self._send( seq, force )

    def __setitem__(self, addr, on):
        """Subscript set operator as shortcut for switch()."""
        self.switch(addr, on)

    def __getitem__(self, addr):
        """Subscript get operator. Returns a switch device instance with the
        correpsonding address."""
        return self.__class__(self._dev, addr)

    def on(self,on=1):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,on)
        
    def off(self):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,0)        


#-----------------------------------------------------------------------


class RS200(mbug_2151_target):
    """RS-200 Remote power system (Europe Supplies/Conrad). This device class represents a
    bunch of addressable switchesor a single switch (if instantiated with given address)."""

    # Transmission parameters
    _timebase = 1e-6
    _iterations = 4
    _timeout = 2
    _seq_type = 'timed_16'
    addr = None

    def __init__(self, dev, addr=None):
        mbug_2151_target.__init__(self, dev)
        self.addr = addr

    def _sequence(self, (syscode, chan), on=1 ):
        # Adresses consist of an 8 bit system code and a switch channel of 1-4 or master (switches all).
        # On the original transmitter, the system code is set at power up by a 4 digit code of 1-4.
        # Here, the system code can be passed as number 0-255 or as the 4 digits code typed on the receiver.
        # Valid switch channels are 1-4 and 'M' for master.
        # Receivers must be learned to an address (after pressing the learn button).
        # Note: The transmitter subsequently sends 2 different packet types, from which only the first type
        # is used here. The second packet type does not seem to have any effect to the tested receivers.
        
        try: syscode = int(syscode)
        except: raise ValueError("Invalid system code")
        if syscode<0 or syscode>4444 or \
           syscode>255 and syscode<1111:
            raise ValueError("Invalid system code")
        if syscode>=1111:
            if min(str(syscode))<'1' or max(str(syscode))>'4':
                raise ValueError("Invalid syscode")
            syscode = sum([ (((syscode//10**i)%10-1)&0b11)<<(6-i*2) for i in range(4) ])
        try: chan = {'1':0, '2':1, '3':2, '4':3, 'M':5, 'A':5}[str(chan).upper()[0]]
        except: raise ValueError("Invalid channel")
        on = int(bool(on))
        cmd012 = chan<<4 | (1<<7)*int(on) | (1<<2)*int(not on) 
        if chan in (1,2): cmd012 ^= 1<<7        
        preamble = 0b011001
        chk012 = (0b1001 + syscode + (syscode>>4) + cmd012 + (cmd012>>4)) & 0b1111
        seq012 = preamble<<20 | syscode<<12 | cmd012<<4 | chk012

        # 2nd packet type. Not used.                
        ##cmd345 = chan<<4 | 0b1100 | (1<<7)*int(not chan in (1,2))  
        ##chk345 = (0b1001 + syscode + (syscode>>4) + cmd345 + (cmd345>>4)) & 0b1111  
        ##seq345 = preamble<<20 | syscode<<12 | cmd345<<4 | chk345  

        seq = []
        for i in range(26):
            seq += [600,3400] if (seq012>>(25-i))&1 else [1300,3400]
        seq[-1] += 30000

        return seq
        
    def switch(self, addr, on, force=0):
        """Turn addressed remote switches on/off. The address of a switch target consists of
        2 numbers: The system code (4 digits 1-4, or number 0-255 alternatively) and a
        receiver address (1-4 or M or A for master). Use with tuple (syscode,addres) as addr, or
        addres only, in which case a previously stored syscode is used (default syscode = 0)."""
        try: addr = (addr[0],addr[1])
        except: addr = (self.addr[0],addr)
        self.addr = addr
        seq = self._sequence(addr, on)
        self._send( seq, force )

    def __setitem__(self, addr, on):
        """Subscript set operator as shortcut for switch()."""
        self.switch(addr, on)

    def __getitem__(self, addr):
        """Subscript get operator. Returns a switch device instance with the
        correpsonding address."""
        return self.__class__(self._dev, addr)

    def on(self,on=1):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,on)
        
    def off(self):
        if self.addr==None: raise ValueError('Target address has not yet been set.')
        self.switch(self.addr,0)        

