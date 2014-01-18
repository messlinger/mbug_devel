
#include <stdlib.h>
#include "mbug_2110.h"

//------------------------------------------------------------------------------
// Get a list of all available mbug-2110 devices. 
const mbug_device_list  mbug_2110_list( void )
{
	return mbug_get_device_list(2110);
}

//------------------------------------------------------------------------------
// Open a device specified by it's serial number (as int, 
// last digits of the serial number are matched only).
mbug_device mbug_2110_open( int serial_num )
{
	return mbug_open_int(2110, serial_num);
}

//------------------------------------------------------------------------------
// Open a device specified by it's id string, as returned
// by mbug_2810_list()
mbug_device mbug_2110_open_str( const char *id )
{
	unsigned int type = mbug_type_from_id( id );
	if (type != 2110) return NULL;
	return mbug_open_str( id );
}

//------------------------------------------------------------------------------
// Close a previously opened device.
void mbug_2110_close( mbug_device dev )
{
	mbug_close(dev);
}

//------------------------------------------------------------------------------
// Read the current GPIO input states (12 bits, little endian bit packed).
// GPIO lines that are not configured for digital input will give undefined results.
int mbug_2110_get_gpio( mbug_device dev )
{
	unsigned char in[8] = {0};

	if (mbug_write( dev, "\xE1", 1) <1)	
		return -1;
	if (mbug_read( dev, in, 2 ) <2)
		return -1;
	return in[0] + (in[1]<<8);
}

//------------------------------------------------------------------------------
// Set the GPIO output states (12 bits, little endian bit packed). States
// will only be applied to GPIO ports that are configured for digital output.
int mbug_2110_set_gpio( mbug_device dev, unsigned int bits )
{
	unsigned char out[8] = {0xE0, 0, 0};
	out[1] = (bits>>0) & 0xff;
	out[2] = (bits>>8) & 0x0f;
	if (mbug_write( dev, out, 3) <3)
		return -1;
	return 0;
}

//------------------------------------------------------------------------------
// Set the GPIO tristate directions (12 bits, little endian bit packed, 
// 0: Output, 1: Input).
int mbug_2110_set_tris( mbug_device dev, unsigned int tris )
{
	unsigned char out[8] = {0xE2, 0, 0};
	out[1] = (tris>>0) & 0xff;
	out[2] = (tris>>8) & 0x0f;
	if (mbug_write( dev, out, 3) <3)
		return -1;
	return 0;
}

//------------------------------------------------------------------------------
// Enable the analog input channels (8 bits, little endian bit packed, 1:enable, 
// 0:disable). ADC channels 0..7 are multiplexed with GPIO channels 3..10.
int mbug_2110_enable_adc( mbug_device dev, unsigned int channels )
{
	unsigned char out[8] = { 0xE8, 0x00, channels&0xFF, (channels>>8)&0xFF, 0};
	if(mbug_write(dev, out, 4) < 4) {
		return -1;
	}
	return 0;
}

//------------------------------------------------------------------------------
// Read all analog input channels (wether enabled or not). ADC resolution is 
// 10 bit (0-1023). Readings are returned as array of 8*16 bit integers.
// A Null pointer is returned in case of an error.
const unsigned short * mbug_2110_read_adc( mbug_device dev )
{
	static unsigned char in[16] = {0};
	if (mbug_write( dev, "\xE9", 1) <1)	
		return NULL;
	if (mbug_read( dev, in, 16 ) <16)
		return NULL;
    return (const unsigned short*) in;
}

//------------------------------------------------------------------------------
// Enable/disable the pwm output on GPIO 0.
int mbug_2110_enable_pwm( mbug_device dev, int channels )
{
	unsigned char out[8] = {0xE4, 0, 0};
	out[1] = (channels>>0) & 0x01;
	if (mbug_write( dev, out, 3) <3)
		return -1;
	return 0;
}

//------------------------------------------------------------------------------
// Set the pwm duty cycle. Duty cycle resolution is 10 bit. 
// The duty cycle can be passed as integer in range 0..1023.
int mbug_2110_set_pwm( mbug_device dev, int duty )
{
	unsigned char out[8] = {0xE5, 0, 0, 0};
	if (duty>1023) duty=1023;
	if (duty<0) duty=0;
	out[2] = (duty>>0) & 0xff;
	out[3] = (duty>>8) & 0xff;
	if (mbug_write( dev, out, 4) <4)
		return -1;
	return 0;
}

//------------------------------------------------------------------------------
