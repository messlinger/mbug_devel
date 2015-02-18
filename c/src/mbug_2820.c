
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
// Open a device specified by it's serial number (as int,
// last digits of the serial number are matched only).
mbug_device mbug_2820_open( unsigned long serial_num )
{
	return mbug_open_int(2820, serial_num);
}

//------------------------------------------------------------------------------
// Open a device specified by it's id string, as returned
// by mbug_2820_list()
mbug_device mbug_2820_open_str( const char *id )
{
	unsigned int type = mbug_type_from_id( id );
	if (type != 2820) return NULL;
	return mbug_open_str( id );
}

//------------------------------------------------------------------------------
// Close a previously opened device.
void mbug_2820_close( mbug_device dev )
{
	mbug_close(dev);
}

//------------------------------------------------------------------------------
// Read temperatures and humidity. Pass a NULL pointer if you are not interested
// in the repective measurement value.
int mbug_2820_read( mbug_device dev, double *temperature, double *humidity )
{
	char data[16] = {0};
	int ret = 0;
	char* endp = 0;

	if (temperature!=0)
		*temperature = NOT_A_TEMPERATURE;
	if (humidity!=0)
		*humidity = NOT_A_TEMPERATURE;

	ret = mbug_write( dev, "\xA0\x00\x01", 3);
	if (ret<3) return -1;

	ret = mbug_read( dev, data, 8 );
	if (ret<8) return -1;
	data[8] = '\0';
	if (temperature!=0) {
		errno = 0;
		*temperature = strtod(data, &endp);
		if (errno || *endp != '\0') {
			*temperature = NOT_A_TEMPERATURE;
			return -1;
		}
	}

	ret = mbug_write( dev, "\xA0\x01\x01", 3);
	if (ret<3) return -1;

	ret = mbug_read( dev, data, 8 );
	if (ret<8) return -1;
	data[8] = '\0';
	if (humidity!=0) {
		errno = 0;
		*humidity = strtod(data, &endp);
		if (errno || *endp != '\0') {
			*humidity = NOT_A_TEMPERATURE;
			return -1;
		}
	}

	return 0;
}


//------------------------------------------------------------------------------
// Read temperature only
double mbug_2820_read_temperature( mbug_device dev )
{
	double temperature;
	mbug_2820_read(dev, &temperature, 0);
	return temperature;
}

//------------------------------------------------------------------------------
// Read humidity only
double mbug_2820_read_humidity( mbug_device dev )
{
	double humidity;
	mbug_2820_read(dev, 0, &humidity);
	return humidity;
}

//------------------------------------------------------------------------------
// Read raw data
long mbug_2820_read_raw( mbug_device dev )
{
	char data[8] = {0};
	int ret;

	ret = mbug_write( dev, "\xA0\x00\x00", 3);
	if (ret<3) return -1;

	ret = mbug_read( dev, data, 8 );
	if (ret<8) return -1;

	return *(long*)data;
}
//------------------------------------------------------------------------------

