
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "mbug_2818.h"

//------------------------------------------------------------------------------
// Get a list of all available mbug_2818 devices.
const mbug_device_list  mbug_2818_list( void )
{
	return mbug_get_device_list(2818);
}

//------------------------------------------------------------------------------
// Init device to default parameters
static void mbug_2818_init( mbug_device dev )
{
	mbug_2818_set_acq_mode(dev, ACQ_MODE_INST);
	mbug_2818_set_tris_bits(dev, 0xFFFF);
	mbug_2818_set_dout_bits(dev, 0x0000);
	mbug_2818_enable_pwm(dev, 0, 0);
}

//------------------------------------------------------------------------------
// Open a device specified by it's serial number (as int,
// last digits of the serial number are matched only).
mbug_device mbug_2818_open( unsigned long serial_num )
{
	mbug_device dev = 
		mbug_open_int(2818, serial_num);
	if (!dev) return 0;

	mbug_2818_init( dev );

	return dev;
}

//------------------------------------------------------------------------------
// Open a device specified by it's id string, as returned
// by mbug_2820_list()
mbug_device mbug_2818_open_str( const char *id )
{
	mbug_device dev = 0;
	unsigned int type = mbug_type_from_id( id );
	if (type != 2818) return 0;

	dev =  mbug_open_str( id );
	if (!dev) return 0;

	mbug_2818_init( dev );
	
	return dev;
}

//------------------------------------------------------------------------------
// Close a previously opened device.
void mbug_2818_close( mbug_device dev )
{
	mbug_close(dev);
}

//------------------------------------------------------------------------------
// Set acquisition mode
int mbug_2818_set_acq_mode( mbug_device dev, mbug_acq_mode mode )
{
	unsigned char cmd = 0;
	switch (mode) {
		case ACQ_MODE_INST: cmd = 0xF2; break;
		case ACQ_MODE_LAST: cmd = 0xF3; break;
		case ACQ_MODE_NEXT: cmd = 0xF4; break;
		case ACQ_MODE_SYNC: cmd = 0xF5; break;
		default: // other modes not implemented
			return -1;
	}
	return mbug_write_byte( dev, cmd );
}

//------------------------------------------------------------------------------
// Read one temperature channel (ascii mode)
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
	if (ret<0) return ret;

	ret = mbug_read( dev, data, 64 );
	if (ret<0) return ret;
	data[63] = '\0';

	ret = 0;
	for (i=0; i<n; i++) {
		char *endp = 0;
		char* a = data + i*8;

		// Channel returns "+inf" or "-inf" on overflow, which is not treated as error
		if ( !strncmp(a, "-inf", 4) )  // -inf typically indicates an open channel (no sensor attached)
			temperatures[i] = -300;    // report as invalid temperature, but do not treat as error 
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
	if (ret<0) return ret;

	ret = mbug_read( dev, data_in, 64 );
	if (ret<0) return ret;

	// Only the first 16 bytes carry values
	for (i=0; i<n; i++)
		data[i] = data_in[i*2] + data_in[i*2+1]*256;

	return 0;
}
//------------------------------------------------------------------------------

// Set the channels I/O direction (tristate, 0: Digital output, 1:Analog input),
// bits packed in one integer
int mbug_2818_set_tris_bits( mbug_device dev, unsigned short tris_bits )
{
	unsigned char cmd[8] = { 0xE2, tris_bits & 0xFF, 0 };
	return mbug_write( dev, cmd, 3 );
}

//------------------------------------------------------------------------------
// Set the channels digital output state (0: low, 1:high),
// bits packed in one integer
int mbug_2818_set_dout_bits( mbug_device dev, unsigned short dout_bits )
{
	unsigned char cmd[8] = { 0xE0, dout_bits & 0xFF, 0 };
	return mbug_write( dev, cmd, 3 );
}

//------------------------------------------------------------------------------
// Enable PWM generation on channel 1 (0: off, 1:on).
// Argument chan is only supplied for upward compatibility and is ignored
// The user must also explicitely configure channel 1 as digital output.
int mbug_2818_enable_pwm( mbug_device dev, unsigned char chan, int enable )
{
	unsigned char cmd[8] = { 0xE4, enable & 0xFF, 0 };
	return mbug_write( dev, cmd, 3 );
}

//------------------------------------------------------------------------------
// Set the PWM duty cycle for channel 1 to duty/1024.
// Argument chan is only supplied for upward comatibility and is ignored.
int mbug_2818_set_pwm( mbug_device dev, unsigned char chan, unsigned short duty )
{
	unsigned char cmd[8] = { 0xE5, 0, 0, 0 };  // For upward compatibility: cmd[1] = channel
	if (duty>1023) duty = 1023;
	cmd[2] = duty & 0xFF;
	cmd[3] = (duty>>8) & 0xFF;
	return mbug_write( dev, cmd, 3 );
}
