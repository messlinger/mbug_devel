
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mbug_2818.h"

//------------------------------------------------------------------------------
// Get a list of all available mbug_2818 devices.
const mbug_device_list  mbug_2818_list( void )
{
	return mbug_get_device_list(2818);
}

//------------------------------------------------------------------------------
// Open a device specified by it's serial number (as int,
// last digits of the serial number are matched only).
mbug_device mbug_2818_open( int serial_num )
{
	return mbug_open_int(2818, serial_num);
}

//------------------------------------------------------------------------------
// Open a device specified by it's id string, as returned
// by mbug_2820_list()
mbug_device mbug_2818_open_str( const char *id )
{
	unsigned int type = mbug_type_from_id( id );
	if (type != 2818) return NULL;
	return mbug_open_str( id );
}

//------------------------------------------------------------------------------
// Close a previously opened device.
void mbug_2818_close( mbug_device dev )
{
	mbug_close(dev);
}

//------------------------------------------------------------------------------
// Set acquisition mode
int mbug_2818_set_acq_mode( mbug_device dev, enum mbug_acquisition_mode mode )
{
	int ret = 0;
	unsigned char cmd = 0;
	switch (mode) {
		case ACQ_MODE_INST: cmd = 0xF2; break;
		case ACQ_MODE_LAST: cmd = 0xF3; break;
		case ACQ_MODE_NEXT: cmd = 0xF4; break;
		case ACQ_MODE_SYNC: cmd = 0xF5; break;
		default: // other modes not implemented
			return -1;
	}
	ret = mbug_write_byte( dev, cmd );
	if (ret<1)  return -1;
	return 0;
}

//------------------------------------------------------------------------------
// Read one temperatures channel (ascii mode)
double mbug_2818_read( mbug_device dev, int channel )
{
	double temperatures[8];
	int ret;

	ret = mbug_2818_read_all(dev, temperatures, 8);
	if (ret<0) return NOT_A_TEMPERATURE;
	if (channel<0 || channel>8) return NOT_A_TEMPERATURE;
	return temperatures[channel];
}


//------------------------------------------------------------------------------
// Read all temperatures (ascii mode)
int mbug_2818_read_all( mbug_device dev, double temperatures[], int n )
{
	char data[64] = {0};
	int i, ret;

	if (n>8) n=8;
	for (i=0; i<n; i++)
		temperatures[i] = -300;

	ret = mbug_write( dev, "\xFB\xA0", 2);
	if (ret<2) return -1;

	ret = mbug_read( dev, data, 64 );
	if (ret<64) return -1;
	data[63] = '\0';

	ret = 0;
	for (i=0; i<n; i++) {
		char *endp = 0;
		char* a = data + i*8;

		// Channel return "+inf" or "-inf" on overflow, which is not treated as error
		if ( !strncmp(a, "-inf", 4) )  // -inf typically indicates an open channel (no sensor)
			temperatures[i] = -300;
		else if ( !strncmp(a, "+inf", 4) )
			temperatures[i] = -log(0);
		else {
			errno = 0;
			temperatures[i] = strtod(a, &endp );
			if (errno || (*endp != ',' && *endp != '\0') ) {
				temperatures[i] = -300;
				ret = -1;  // Report error if at least one value is not valid
			}
		}
	}

	return ret;
}

//------------------------------------------------------------------------------
// Read raw data
int mbug_2818_read_raw( mbug_device dev, unsigned short data[], int n )
{
	char data_in[64] = {0};
	int i, ret;

	if (n>8) n=8;
	for (i=0; i<n; i++)
		data[i] = 0;

	ret = mbug_write( dev, "\xFA\xA0", 2);
	if (ret<2) return -1;

	ret = mbug_read( dev, data_in, 64 );
	if (ret<64) return -1;

	// Only the first 16 bytes carry values
	for (i=0; i<n; i++)
	{
		data[i] = data_in[i*2]+data_in[i*2+1]*256;
	}

	return 0;
}
//------------------------------------------------------------------------------

