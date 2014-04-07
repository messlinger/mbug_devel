

#-----------------------------------------------------------------
from threading import Lock as _Lock
from . import libusb0 as usb
#-----------------------------------------------------------------

def mbug_list(type=None):
    """Return a list of all attached MBUG devices of a certain type."""
    found = []
    for dev in usb.get_device_list():
        desc = dev.descriptor
        if desc.idVendor==0x04D8 \
           and desc.idProduct==0xFBC3\
           and (type==None or desc.bcdDevice==_bcd(type)):
                handle = usb.open(dev)
                try:
                    ## s = usb.get_string( handle, desc.iSerialNumber )
                    s = _repeat( usb.get_string, handle, desc.iSerialNumber  )
                    if s!=None: found.append(s)
                finally:
                    usb.close(handle)   
    return found

list = mbug_list
list_devices = mbug_list


#==================================================================
class mbug(object):
    """ Represents a generic MBUG device """
    
    def __init__(self, type=None, serial=None):
        """Open an MBGUG device with specified serial number and type.
           The serial number can be passed as string or integer.
           It is enough to specifiy the significant ending of the serial number.
           If the serial number is not specified, return the first device found.
        """
        self._handle = None
        self._io__Lock = _Lock()
        self._open(type, serial)
        

    def __del__(self):
        self.close()

    def _open(self, type=None, serial=None):
        if self._handle != None:
            self._close()
        for dev in usb.get_device_list():
            desc = dev.descriptor
            if desc.idVendor==0x04D8 \
               and desc.idProduct==0xFBC3 \
               and (type==None or desc.bcdDevice==_bcd(type)) \
               and (serial==None or desc.iSerialNumber!=0):
                with self._io__Lock:
                    handle = usb.open(dev)  # Need to open the device to query string descriptors
                    try:    
                        ## s = usb.get_string( handle, desc.iSerialNumber )
                        s = _repeat( usb.get_string, handle, desc.iSerialNumber )
                        if (serial==None or s.endswith( str(serial) )):
                            ##usb.set_configuration( handle, 1 )
                            _repeat( usb.set_configuration, handle, 0 )  # Workaround for xHCI bug
                            _repeat( usb.set_configuration, handle, 1 ) 
                            _repeat( usb.claim_interface, handle, 0 )
                            self._handle = handle
                            self._serial = s
                            self._type = _bin(desc.bcdDevice)
                            nepts = dev.config[0].interface[0].altsetting[0].bNumEndpoints
                            for i in range(nepts):
                                ep = dev.config[0].interface[0].altsetting[0].\
                                        endpoint[i].bEndpointAddress
                                size = dev.config[0].interface[0].altsetting[0].\
                                        endpoint[i].wMaxPacketSize
                                if (ep&0x80): self._ep_in, self._size_in = ep, size
                                else: self._ep_out, self._size_out = ep, size
                            return
                    except:
                        usb.close(handle)
                        raise
        raise Exception('Device not found.')

    def close(self):
        """ Close the device. Communication is not possible afterwards. """
        if self._handle != None:
            usb.close( self._handle )
            self._handle = None
            self._serial = ''
            self._type = 0
            self._ep_in, self._size_in = 0, 0
            self._ep_out, self._size_out = 0, 0

    def serial(self):
        return self._serial
    def type(self):
        return self._type

    def _write(self, data):
        """Write raw data to the device. Data should be list-like, a string (bytes or 
        unicode) or a single integer. Data is automatically cropped or filled to the 
        required endpoint size.  """
        if self._handle is None:
            raise Exception("Invalid device.")
        try: iter(data)      # Try to iterate.
        except TypeError: data = (data,)  # Convert to tuple.
        if isinstance(data, str): data = data.encode()  # Convert str to bytes 
        with self._io__Lock:
            usb.interrupt_write( self._handle, self._ep_out, bytearray(data), self._size_out )

    def _read(self):
        """ Read raw data from the device."""
        if self._handle is None:
            raise Exception("Invalid device")
        data = usb.create_data_buffer(self._size_in)
        with self._io__Lock:
            usb.interrupt_read( self._handle, self._ep_in, data, self._size_in )
        return bytearray(data)

#----------------------------------------------------------------------
# Helper function. Convert integer to 4 digit bcd format and vice versa.
# Intended for usage with bcd format descriptor fields.
def _bcd(bin):
    return sum([ ((int(bin)//10**d)%10 )*(1<<4*d) for d in range(4) ])
def _bin(bcd):
    return sum([ ((int(bcd)//16**d)%16 )*(10**d) for d in range(4) ])
    
#----------------------------------------------------------------------
# Helper function. Try call to libusb function several times.
# Low speed mbugs sometimes fail with control messages, only happens on some USB hubs.    
def _repeat( func, *args ):
    nrep = 50
    for i in range(nrep):
        try: 
            res = func( *args )
            if (isinstance(res,int) and res>=0) or res!=None: break 
        except usb.LibusbError:
            if i==nrep-1: raise
    return res


#======================================================================

def mbug_test( typ=None ):
    if typ==None:
        devs = list_devices(None)
        print("Attached devices:")
        print(devs if devs!=[] else "None")
    for t in mbug_types if typ==None else typ:
        exec("from . import mbug_%.4d" % t)
        exec("mbug_%.4d.mbug_%.4d_test()" % (t,t) )


test = mbug_test
    
#----------------------------------------------------------------------
if __name__ == "__main__":
    mbug_test()
