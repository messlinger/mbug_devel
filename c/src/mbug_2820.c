
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mbug_2820.h"

//------------------------------------------------------------------------------
// Get a list of all available mbug-2820 devices.
const mbug_device_list  mbug_2820_list( void )
{
	return mbug_get_device_list(2820);
}

//------------------------------------------------------------------------------
// Init device to default parameters
static void mbug_2820_init( mbug_device dev )
{
	mbug_2820_set_acq_mode(dev, ACQ_MODE_INST);
}

//------------------------------------------------------------------------------
// Open a device specified by it's serial number (as int, 
// last digits of the serial number are matched only).
mbug_device mbug_2820_open( unsigned long serial_num )
{
	mbug_device dev = 
		mbug_open_int(2820, serial_num);
	if (!dev) return 0;

	mbug_2820_init( dev );

	return dev;
}

//------------------------------------------------------------------------------
// Open a device specified by it's id string, as returned
// by mbug_2820_list()
mbug_device mbug_2820_open_str( const char *id )
{
	mbug_device dev = 0;
	unsigned int type = mbug_type_from_id( id );
	if (type != 2820) return NULL;

	dev =  mbug_open_str( id );
	if (!dev) return 0;

	mbug_2820_init( dev );
	
	return dev;
}

//------------------------------------------------------------------------------
// Close a previously opened device.
void mbug_2820_close( mbug_device dev )
{
	mbug_close(dev);
}

//------------------------------------------------------------------------------
// Read raw data
long mbug_2820_read_raw( mbug_device dev )
{
	char data[8] = {0};
	int ret;

	ret = mbug_write( dev, "\xA0\x00\x00", 3);
	if (ret<0) return -1;

	ret = mbug_read( dev, data, 8 );
	if (ret<0) return -1;

	return *(long*)data;
}

//------------------------------------------------------------------------------
// Read temperatures and humidity. Pass a NULL pointer if you are not interested
// in the repective measurement value.
int mbug_2820_read( mbug_device dev, double *temperature, double *humidity )
{
	char data[16] = {0};
	int ret = 0;
	char* endp = 0;

	if (temperature != 0) *temperature = -300;
	if (humidity != 0) *humidity = -300;

	// Read 1st data packet
	ret = mbug_write( dev, "\xA0\x00\x01", 3);
	if (ret<0) return -1;
	ret = mbug_read( dev, data, 8 );
	if (ret<0) return -1;

	// Read 2nd data packet
	ret = mbug_write( dev, "\xA0\x01\x01", 3);
	if (ret<0) return -1;
	ret = mbug_read( dev, data+8, 8 );
	if (ret<0) return -1;

	// Convert temperature
	ret = 0;
	data[7] = '\0';
	if (temperature != 0) {
		errno = 0;
		*temperature = strtod(data, &endp);
		if (errno || *endp != '\0') {
			*temperature = -300;
			return -1;
		}
	}

	// Convert humidty
	data[15] = '\0';
	if (humidity != 0) {
		errno = 0;
		*humidity = strtod(data+8, &endp);
		if (errno || *endp != '\0') {
			*humidity = -300;
			return -1;
		}
	}

	// Test for device errors. Specific error code is located at data[5:6] as hex ascii.
	if (*temperature <= NOT_A_TEMPERATURE && data[4]=='\0' && data[5]!='\0') {
		*humidity = *temperature = -300 - strtol(data+5, 0, 16); // until an extended error handler is written: just add
		ret = -1;
	}

	return ret;
}


//------------------------------------------------------------------------------
// Read temperature only
double mbug_2820_read_temperature( mbug_device dev )
{
	double temperature = -300;
	mbug_2820_read(dev, &temperature, 0);
	return temperature;
}

//------------------------------------------------------------------------------
// Read humidity only
double mbug_2820_read_humidity( mbug_device dev )
{
	double humidity = -300;
	mbug_2820_read(dev, 0, &humidity);
	return humidity;
}

//------------------------------------------------------------------------------
// Set the acquisition mode (see mbug.h for documentation)
int mbug_2820_set_acq_mode( mbug_device dev, enum mbug_acquisition_mode mode )
{
	unsigned char cmd[2] = {0xF2, 0};
	switch (mode) {
		case ACQ_MODE_INST: cmd[1] = 0x00; break;
		case ACQ_MODE_LAST: cmd[1] = 0x01; break;
		case ACQ_MODE_NEXT: cmd[1] = 0x02; break;
		case ACQ_MODE_SYNC: cmd[1] = 0x03; break;
		default: // other modes not implemented
			return -1;
	}
	return mbug_write( dev, cmd, 2 );
}
//------------------------------------------------------------------------------
