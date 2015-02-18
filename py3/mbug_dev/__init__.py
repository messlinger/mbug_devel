

#-----------------------------------------------------------------
from .mbug_base import *
#-----------------------------------------------------------------

# Moved all device classes to separate files. Import them all.
mbug_types = [ 1220, 2110, 2151, 2165, 2810, 2811, 2818, 2820 ]
types = mbug_types

for _t in mbug_types:
    exec( "from .mbug_%.4d import mbug_%.4d" % (_t,_t) )
    
from . import mbug_2151_target

#-----------------------------------------------------------------

__all__ = [
    'mbug_list', 'mbug_open', 'mbug', 'mbug_test',
    ] + ["mbug_%.4d" % _t for _t in mbug_types ]

#-----------------------------------------------------------------

def mbug_open(type, serial=None):
    """Open and returns an MBUG device with specified serial number and type.
       The serial number can be passed as string or integer. It is enough to 
       specifiy the significant ending of the serial number. If the serial 
       number is not specified, return the first device found.
       The device can also be specified as string 'MBUG-TTTT-SSSSSS' with
       T=type, S=serial, which is the format of the mbug's USB iSerialNumber
       string and is also returned by mbug.list() in this form.
       The function returns a device class specific to the corresponding 
       device type.
    """
    try: type = int(type)
    except:
        try:
            st = str(type).split('-')
            if not st[0].upper().startswith('MBUG'): raise
            type = int(st[1])
            serial = int(st[2])
        except:
            raise Exception('Invalid device id.')
    if not type in mbug_types:
        raise Exception("Invalid device type.")
    cls = eval("mbug_"+str(type))
    return cls(serial)

open = mbug_open
open_device = mbug_open

#-----------------------------------------------------------------