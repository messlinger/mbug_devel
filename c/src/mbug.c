
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#ifdef _WIN32
	#include "lusb0_usb.h"	// has been renamed in v 1.2.5
#else
	#include "usb.h"
#endif

#include "mbug.h"

#ifdef _MSC_VER
	#pragma warning(disable:4200)
	#pragma warning(disable:4996)
#endif

//-----------------------------------------------------------------------------------------

#define VID	 0x04D8	  // Vendor id (Microchip)
#define PID	 0xFBC3	  // Product id (mbug subvendor license)

#define MBUG_MAX_DEVICES		255
#define MBUG_MAX_ID_LEN		    32

#define MBUG_TIMEOUT			5000 // (ms)

#define MBUG_IGNORE_STR_ERR     1

//-----------------------------------------------------------------------------------------

struct mbug_device_struct {
	// Note: libusb uses two structs for device representation.
	// device* is used for information and descriptor retrieving.
	// device_handle* is used for control and i/o.
	struct usb_device* usb_device;
	struct usb_dev_handle* usb_dev_handle;
	int device_type;
	char id[MBUG_MAX_ID_LEN+1];
	int timeout;
	int ep_out, ep_in;
	int size_out, size_in;
	void* aux;	// Auxiliary data, managed by device specific libraries 
};


//----------------------------------------------------------------------
// Helper functions. Convert integer to 4 digit bcd format and vice versa.
// Intended for usage with bcd format descriptor fields.
unsigned short _bcd(unsigned short bin)
{
    return ((bin/1)%10)*(1<<0) + ((bin/10)%10)*(1<<4) 
		+ ((bin/100)%10)*(1<<8) + ((bin/1000)%10)*(1<<12);
}
unsigned short _bin(unsigned short bcd)
{
	return ((bcd>>0)%16 )*(1) + ((bcd>>4)%16 )*(10)
		+ ((bcd>>8)%16 )*(100) + ((bcd>>12)%16 )*(1000);
}

//-----------------------------------------------------------------------------------------

const mbug_device_list mbug_get_device_list(unsigned int device_type)
{
	static char* device_list[MBUG_MAX_DEVICES+1];
	static char device_ids[MBUG_MAX_DEVICES+1][MBUG_MAX_ID_LEN+1];

	struct usb_bus* bus = 0;
	usb_dev_handle* handle = 0;
	int idev = 0;
	char id_str[MBUG_MAX_ID_LEN] = "";
	int ret = 0;
	int i;
	
	memset( (void*)device_list, 0, sizeof(device_list) );
	memset( device_ids, 0, sizeof(device_ids) );

	usb_init();
    usb_find_busses();
    usb_find_devices();

	// Iterate through all available devices
	for ( bus = usb_get_busses(); bus; bus = bus->next)
	{
		struct usb_device *dev;
        for (dev = bus->devices; dev; dev = dev->next)
		{
			// VID:PID match?
			if (VID != dev->descriptor.idVendor ||
				PID != dev->descriptor.idProduct)
				continue;

			// Device_type match?
			if (device_type != 0 &&
				_bcd(device_type) != dev->descriptor.bcdDevice )
				continue;

			// Serial number string defined?
			if (dev->descriptor.iSerialNumber == 0)
				continue;

			// Need to obtain a device handle to query string descriptors
			handle = usb_open( dev );
			if (handle<=0) continue;

			// Read the iSerialNumber string
			ret = usb_get_string_simple( handle, dev->descriptor.iSerialNumber,
				                             id_str, sizeof(id_str)  );
			// :XXX:
			// Low speed mbugs sometimes give errors when querying string descriptors:
			// -5 sending control message failed. This only happens with some USB hubs.
			// Workaround: Try several times
			for (i=0; ret<0 && i<50; i++)
				ret = usb_get_string_simple( handle, dev->descriptor.iSerialNumber,
				                             id_str, sizeof(id_str)  );
			#if MBUG_IGNORE_STR_ERR
				if (ret<0)
					strncpy(id_str, "(Unidentified device)", MBUG_MAX_ID_LEN);
			#else
				if (ret<0) goto close_and_continue;
			#endif

			// Match! Add device to list.
			strncpy( device_ids[idev], id_str, MBUG_MAX_ID_LEN );
			device_list[idev] = device_ids[idev];
			if (++idev>=MBUG_MAX_DEVICES) break;

		close_and_continue:
			usb_close( handle );
			continue;
		}
	}

	return (const mbug_device_list) device_list;
}

//-----------------------------------------------------------------------------------------
// Open by type and serial number. serial_num=0 will open the first available device.
mbug_device mbug_open_int( unsigned int device_type, unsigned long serial_num )
{
	struct usb_bus* bus =0 ;
	struct usb_device *dev = 0;
	usb_dev_handle* handle = 0;
	char id_str[MBUG_MAX_ID_LEN] = "";
	int ret = 0;
	int nepts = 0;
	int i = 0;
	mbug_device_struct* mbug_dev = 0; 

	usb_init();
	usb_find_busses();
    usb_find_devices();

	// Iterate through all available devices
	for (bus = usb_get_busses(); bus; bus = bus->next)
	{
		
        for (dev = bus->devices; dev; dev = dev->next)
		{
			//  VID:PID match?
			if (VID != dev->descriptor.idVendor ||
				PID != dev->descriptor.idProduct )
				continue;

			// Device_type match?
			if ( _bcd(device_type) != dev->descriptor.bcdDevice )
				continue;

			// Serial number string defined?
			if (dev->descriptor.iSerialNumber == 0)
				continue;

			// Need to obtain a device handle to query string descriptors
			handle = usb_open( dev );
			if (handle<=0) continue;

			// Read the iSerialNumber string
			ret = usb_get_string_simple( handle, dev->descriptor.iSerialNumber,
				                             id_str, sizeof(id_str)  );
			// :XXX:
			// Low speed mbugs sometimes give errors when querying string descriptors:
			// -5 sending control message failed. This only happens with some USB hubs.
			// Workaround: Try several times
			for (i=0; ret<0 && i<50; i++)
				ret = usb_get_string_simple( handle, dev->descriptor.iSerialNumber,
				                             id_str, sizeof(id_str)  );
			#if MBUG_IGNORE_STR_ERR
				if (ret<0)  // Could not identify the device
					if(serial_num==0)  // If the user specified no serial number, accept it even though
						strncpy(id_str, "(Unidentified device)", MBUG_MAX_ID_LEN);
					else goto close_and_continue;
			#else
				if (ret<0) goto close_and_continue;
				
				// Compare id string.
				if (serial_num != 0 &&
					serial_num != atoi(id_str+strlen(id_str)-6) )
					goto close_and_continue;
			#endif

			// Everything matches, so this is our device.
			// But we have to initialize it first.

			// Detach a previously installed kernel driver
			// (Linux only, will have no effect on windows.)
			#if LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
				usb_detach_kernel_driver_np(handle, 0);
				if (ret<0) goto close_and_continue;
			#endif

			// Set device to configuration 1
			ret = usb_set_configuration( handle, 1 );
			if (ret<0) goto close_and_continue;

			// Claim the first interface
			ret = usb_claim_interface( handle, 0 );
			if (ret<0) goto close_and_continue;

			// Everything seems fine so far, so
			// we consider the device as opened.

			// Create mbug device struct and copy some data for later use
			mbug_dev = (mbug_device_struct*) 
							calloc(1,sizeof(mbug_device_struct));
			mbug_dev->usb_device = dev;
			mbug_dev->usb_dev_handle = handle;
			mbug_dev->device_type = _bin(device_type);
			strncpy( mbug_dev->id, id_str, sizeof(mbug_dev->id) );
			mbug_dev->timeout = MBUG_TIMEOUT;
			mbug_dev->aux = 0;
			// Parse endpoints
			nepts = dev->config[0].interface[0].altsetting[0].bNumEndpoints;
            for (i=0; i<nepts; i++)
			{
				int ep = dev->config[0].interface[0].altsetting[0].
							endpoint[i].bEndpointAddress;
                int size = dev->config[0].interface[0].altsetting[0].\
							endpoint[i].wMaxPacketSize;
				if (ep&0x80) 
				{
					mbug_dev->ep_in = ep;
					mbug_dev->size_in = size;
				} 
				else 
				{
					mbug_dev->ep_out = ep;
					mbug_dev->size_out = size;
				}
			}

			// Make shure all endpoints are properly recognized
			if (mbug_dev->ep_out==0 || mbug_dev->ep_in==0 ||
				mbug_dev->size_out==0 || mbug_dev->size_in==0)
				goto close_and_continue;

			return mbug_dev;

		close_and_continue:
			usb_close( handle );
			continue;
		}
	}

	// No matching device was found
	return NULL;
}

//-----------------------------------------------------------------------------------------
// Open by id string ("MBUG-TTTT-SSSSSS"). The leading "MBUG-" may be omitted.
mbug_device mbug_open_str( const char* id_str )
{
	unsigned int device_type = 0;
	unsigned long serial_num = 0;
	device_type = mbug_type_from_id( id_str );
	if (device_type==0)
		return NULL;
	serial_num = mbug_serial_from_id( id_str );
	if (serial_num < 0)
		return NULL;
	return mbug_open_int( device_type, serial_num );
}

//-----------------------------------------------------------------------------------------
// Extract the device type from a given id string
// Return value < 0 indiates an invalid id 
int mbug_type_from_id( const char *id )
{
	unsigned int device_type = 0;
	char *endp = 0;
	if (!strncmp( id, "MBUG-", 5 ))
		id += 5;
	if (strlen(id)!=11 || id[4]!='-')
		return -1;
	device_type = strtoul( id, &endp, 10 );
	if (*endp != '\0' || device_type<1000)
		return -1;
	return device_type;
}

// Extract the serial number from a given id string
// Return value < 0 indiates an invalid id 
long mbug_serial_from_id( const char *id )
{
	unsigned long serial_num = 0;
	char *endp = 0;
	if (!strncmp( id, "MBUG-", 5 ))
		id += 5;
	if (strlen(id)!=11 || id[4]!='-')
		return -1;
	serial_num = strtoul( id+5, &endp, 10 );
	if (*endp!='\0')
		return -1;
	return serial_num;
}

//-----------------------------------------------------------------------------------------

void mbug_close( mbug_device dev )
{
	if (dev==0) return;
	if (dev->usb_dev_handle != 0)
		usb_close(dev->usb_dev_handle);
	if (dev->aux != 0)
		free( dev->aux );
	free(dev);
	return;
}

//-----------------------------------------------------------------------------------------

int mbug_read( mbug_device dev, void* data, int size )
{
	int ret = 0;
	char data_in[64] = {0};		// (maximum packet size for full speed interrupt/bulk endpoints)
	
	if (dev==0) return -1;
	if (size > dev->size_in) 
		size = dev->size_in;
	
	ret = usb_interrupt_read(
		dev->usb_dev_handle,
		dev->ep_in,
		data_in,
		dev->size_in,
		dev->timeout);

	if (ret<0) return -1;
	
	memcpy( data, data_in, size );
	return ret;
}

//-----------------------------------------------------------------------------------------

int mbug_write( mbug_device dev, const void* data, int size )
{
	int ret = 0;
	char data_out[64] = {0};

	if (dev==0) return -1;
	if (size > dev->size_out) 
		size = dev->size_out;

	memcpy(data_out, data, size);
	
	ret = usb_interrupt_write(
		dev->usb_dev_handle,
		dev->ep_out,
		data_out,
		dev->size_out,
		dev->timeout);
	return ret;
}

//-----------------------------------------------------------------------------------------

const char* mbug_error( mbug_device dev )
{
	// Actually, libusb-0 only provides a global error message.
	// Device handle is not needed.
	return usb_strerror();
}


