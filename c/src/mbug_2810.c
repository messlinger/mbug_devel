
#include <stdlib.h>
#include <errno.h>
#include "mbug_2810.h"

//------------------------------------------------------------------------------
// Get a list of all available mbug-2810 devices. 
const mbug_device_list  mbug_2810_list( void )
{
	return mbug_get_device_list(2810);
}

//------------------------------------------------------------------------------
// Init device to default parameters
static void mbug_2810_init( mbug_device dev )
{
	mbug_2810_set_acq_mode(dev, ACQ_MODE_INST);
}

//------------------------------------------------------------------------------
// Open a device specified by it's serial number (as int, 
// last digits of the serial number are matched only).
mbug_device mbug_2810_open( unsigned long serial_num )
{
	mbug_device dev = 
		mbug_open_int(2810, serial_num);
	if (!dev) return 0;

	mbug_2810_init( dev );

	return dev;
}

//------------------------------------------------------------------------------
// Open a device specified by it's id string, as returned
// by mbug_2810_list()
mbug_device mbug_2810_open_str( const char *id )
{
	mbug_device dev = 0;
	unsigned int type = mbug_type_from_id( id );
	if (type != 2810) return NULL;

	dev =  mbug_open_str( id );
	if (!dev) return 0;

	mbug_2810_init( dev );
	
	return dev;
}

//------------------------------------------------------------------------------
// Close a previously opened device.
void mbug_2810_close( mbug_device dev )
{
	mbug_close(dev);
}

//------------------------------------------------------------------------------
// Read raw sensor value
int mbug_2810_read_raw( mbug_device dev )
{
	unsigned char in[8] = {0};
	int ret = 0;

	ret = mbug_write( dev, "\xFA\xA0\xF2", 3);	// Raw format (0xF2 is the old revisions read cmd)
	if (ret<0) return -1;

	ret = mbug_read( dev, in, 8 );
	if (ret<0)	return -1;

	return in[0]+(in[1]<<8);
}

//------------------------------------------------------------------------------
// Read the current temperature (ascii mode). 
// Returns out of band temperature <= NOT_A_TEMPERATURE in case of errors.
double mbug_2810_read_ascii( mbug_device dev )
{
	char str[8] = {0};
	double tem = -300;
	char *endp;
	int ret = 0;

	ret = mbug_write( dev, "read", 5);
	if (ret<0) return -300;

	ret = mbug_read( dev, str, 8 );
	if (ret<0) return -300;

	str[7] = 0;  // Just to be sure	
	errno = 0;
	tem = strtod( str, &endp );
	if (errno || *endp != '\0' )
		return -300;

	// Data was succesfully received and converted. There might still be a device error,
	//    indicated by an out-of-band value < NOT_A_TEMPERATURE.
	if (tem <= NOT_A_TEMPERATURE && str[4]=='\0' && str[5]!='\0')    
									// Specific error code is located in str[5:6] as hex ascii
		tem -= strtol(str+5, 0, 16); // until an extended error handler is written: just add

	return tem;
}

//------------------------------------------------------------------------------

// Default read mode is ascii
double mbug_2810_read( mbug_device dev )
{
	return mbug_2810_read_ascii( dev );
}

//------------------------------------------------------------------------------
// Set the acquisition mode (see mbug.h for documentation)
int mbug_2810_set_acq_mode( mbug_device dev, mbug_acq_mode mode )
{
	unsigned char cmd = 0;
	switch (mode) {
		case ACQ_MODE_INST: cmd = 0xF2; break;
		case ACQ_MODE_LAST: cmd = 0xF3; break;
		case ACQ_MODE_NEXT: cmd = 0xF4; break;
		default: // other modes not implemented
			return -1;
	}
	return mbug_write_byte( dev, cmd );
}

