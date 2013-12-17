

def _sequence( (syscode, channels), on=None, level=None, rel=None, gradual=0):
    
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
    d += [(syscode>>i)&1 for i in (0,1,2,3)]
    d += [int(i in channels) for i in range(10)]
    d += [1^d[-10]^d[-8]^d[-6]^d[-4]^d[-2], 1^d[-9]^d[-7]^d[-5]^d[-3]^d[-1]]
    d += [(cmd>>i)&1 for i in (0,1,2,3,4)]
    d += [int(gradual)]
    d += [1^d[-6]^d[-4]^d[-2], 1^d[-5]^d[-3]^d[-1]]

    # biphase-mark-encoding
    seq = [1,1^d[0]]
    for b in d[1:]:
        seq += [seq[-1]^1, seq[-1]^1^b]

    return d,seq
    

def switch(self, addr, on, force=0):
    try: addr = (addr[0],addr[1])
    except: addr = (self.syscode,addr)
    self.addr = addr
    seq = self._sequence(addr, on)
    self._send( seq, force )
    
def set_level(self, addr, level, force=0):
    try: addr = (addr[0],addr[1])
    except: addr = (self.syscode,addr)
    self.addr = addr
    seq = self._sequence(addr, level)
    self._send( seq, force )


