
//==============================================================================
/* MBUG-2151 interface implementation
		
		Mostly identical to mbug_2165.c, but without carrier modulation.
		Any changes in this file should also be applied in mbug_2165.c.
*/
//==============================================================================

#include <stdlib.h>
#include <string.h>
#include "mbug_2151.h"

//------------------------------------------------------------------------------
// Parameter numbers (keep private, may change in future revisions)

enum  mbug_2151_param {
	PARAM_START = 0x10,
	PARAM_RESET = 0xFF,
	PARAM_ITER  = 0x01,
	PARAM_CLOCK_DIV = 0x02,
	PARAM_BASE_CLOCK = 0x05,
	PARAM_BUSY = 0x70,
	PARAM_SEQ_LEN = 0x71
};

//------------------------------------------------------------------------------
// Get a list of all available mbug-2151 devices. 
const mbug_device_list  mbug_2151_list( void )
{
	return mbug_get_device_list(2151);
}

//------------------------------------------------------------------------------
// Device base clock.  This is assumed to be identical for
//   all mbug-2151 devices and only read the very first time a device is opened.
//   Currently, _base_clock = _mod_clock = 4 MHz.

static int base_clock = 0;	

//------------------------------------------------------------------------------
// Open a device specified by it's serial number (as int, 
// last digits of the serial number are matched only).
mbug_device mbug_2151_open( int serial_num )
{
	mbug_device dev = mbug_open_int(2151, serial_num);
	if ( (dev != NULL) && (base_clock == 0) )
	{
		base_clock = mbug_2151_get_base_clock( dev );
	}
	return dev;
}

//------------------------------------------------------------------------------
// Open a device specified by it's id string, as returned
// by mbug_2151_list()
mbug_device mbug_2151_open_str( const char *id )
{
	mbug_device dev;
	unsigned int type = mbug_type_from_id( id );
	if (type != 2151) return NULL;
	dev = mbug_open_str( id );
	if ( (dev != NULL) && (base_clock == 0) )
	{
		base_clock = mbug_2151_get_base_clock( dev );
	}
	return dev;
}

//------------------------------------------------------------------------------
// Close a previously opened device.
void mbug_2151_close( mbug_device dev )
{
	mbug_close(dev);
}

//------------------------------------------------------------------------------
// Private helper function:  
//   Generic I/O command: Send cmd, receive integer parameter
//   Return values <0 indicate errors. 
static int io( mbug_device dev,  unsigned char* out,  size_t size )
{
	unsigned char in[8] = {0};
	short status = 0;
	int value = 0;

	if (mbug_write( dev, out, size ) <0)
		return -1;
	if (mbug_read( dev, in, sizeof(in) ) <0)
		return -1;
	
	// First 2 bytes contain the return status (signed 16 bit)
	status = *(short*) in;
	// Next 4 bytes contain the returned output value
	value = *(int*) (in+2);
	if (status<0)  return status;
	else  return value;
}

//------------------------------------------------------------------------------
// Private helper function:  
//   Set parameter value. 
static int set_param( mbug_device dev, enum mbug_2151_param param, unsigned int value )
{
	unsigned char out[8] = {0xB0, param, 0, 0, 0, 0, 0, 0};
	memcpy( out+2, (void*)&value, 4 );
	return io( dev, out, 6);
}

//------------------------------------------------------------------------------
// Private helper function:  
//   Get parameter value. 
static int get_param( mbug_device dev, enum mbug_2151_param param )
{
	unsigned char out[2] = {0xC0, param};
	return io( dev, out, 2);
}

//------------------------------------------------------------------------------
// Set transmission sequence (max 255 bytes)
int mbug_2151_set_sequence( mbug_device dev, unsigned char* data, size_t nbytes, mbug_2151_tx_mode tx_mode )
{
	unsigned char out[64] = {0xB0, 0};
	unsigned char* pd = data;
	size_t  s, smax; 

	if (nbytes>255)  nbytes=255;
	if (tx_mode > TX_MODE_TIMED_16)  
		tx_mode = TX_MODE_BITSTREAM;

	smax = 64 - 4;  // Maximum data length per packet

	// First packet: Set sequence	
	// Command format: {0xB0, 0, mode<<4, len, data[:] }
	s = nbytes>smax ? smax : nbytes;
	out[2] = tx_mode<<4;  out[3] = s;
	memcpy( out+4, pd, s );
	nbytes -= s;  pd += s;
	if (io( dev, out, s+4 ) <0)
		return -1;

	// Subsequent packages: Append sequence
	// Command format: {0xB0, 0, 0x01, len, data[] }
	while (nbytes>0)
	{
		s = nbytes>smax ? smax : nbytes;
		out[2] = 0x01;  out[3] = s;
		memcpy( out+4, pd, s );
		nbytes -= s;  pd += s;
		if (io( dev, out, s+4 ) <0)
			return -1;
	}
	return 0;
}

//------------------------------------------------------------------------------
// Set transmission sequence with 16-bit values (max 127 items)
int mbug_2151_set_sequence_16bit( mbug_device dev, unsigned short* data, size_t nitems )
{
	return mbug_2151_set_sequence( dev, (unsigned char*)data, nitems*2, TX_MODE_TIMED_16 );
}


//------------------------------------------------------------------------------
// Get the size (in bytes) of the currently set sequence
int mbug_2151_get_seq_length( mbug_device dev )
{
	return get_param( dev, PARAM_SEQ_LEN );
}

//------------------------------------------------------------------------------
// Set number of transmission sequence iterations, 
// 0: infinite, Maximum: 2**16-1
int mbug_2151_set_iterations( mbug_device dev, unsigned int niter )
{
	if (niter >= 1<<16 )  niter = (1<<16)-1;
	return set_param( dev, PARAM_ITER, niter );  
}

//------------------------------------------------------------------------------
// Get the currently set number of transmission sequence iterations, 
// 0: infinite
int mbug_2151_get_iterations( mbug_device dev )
{
	return get_param( dev, PARAM_ITER );
}

//------------------------------------------------------------------------------
// Get the devices base transmission clock frequency in Hz
int mbug_2151_get_base_clock( mbug_device dev )
{	
	return get_param( dev, PARAM_BASE_CLOCK );
}

//------------------------------------------------------------------------------
// Set clock divider. => Bitrate = base_clock/div
// Minimum div is 10 for bitstream mode, 1 for timed modes, 
// Maximum div is 2**16-1 (65535).
int mbug_2151_set_clock_div( mbug_device dev, unsigned int div )
{
	if (div == 0)  div = 1;
	if (div >= 1<<16 )  div = (1<<16)-1;
	return set_param( dev, PARAM_CLOCK_DIV, div );
}

//------------------------------------------------------------------------------
// Get the currently set transmission clock divider.
int mbug_2151_get_clock_div( mbug_device dev )
{
	return get_param( dev, PARAM_CLOCK_DIV );
}

//------------------------------------------------------------------------------
// Set the transmission rate (via clock div).
// Minimum rate is 61sec.
// Maximum rate is 400kHz for bitstream mode, 4Mhz for timed modes.
// Returns the actually set bitrate or <0 in case of an error.
double mbug_2151_set_bitrate( mbug_device dev, double freq )
{
	int div;
	div = (int)(0.5 + 1.0*base_clock/freq);
	if (div >= 1<<16)  div = (1<<16)-1;
	if( mbug_2151_set_clock_div( dev, div ) < 0)
		return -1;
	return (int)(0.5 + 1.0*base_clock/div);
}

//------------------------------------------------------------------------------
// Set the transmission base interval (in seconds, via clock div).
// Minimum base interval is 2.5us for bitstream mode, 0.25us for timed modes.
// Maximum base interval is 16.38375ms.
// Resolution is 0.25us.
// Returns the actually set interval or <0 in case of an error.
double mbug_2151_set_interval( mbug_device dev, double interval )
{
	int div;
	div = (int)(0.5 + 1.0*interval*base_clock);
	if (div >= 1<<16)  div = (1<<16)-1;
	if( mbug_2151_set_clock_div( dev, div ) < 0)
		return -1;
	return (int)(0.5 + 1.0*div/base_clock);
}

//------------------------------------------------------------------------------
// Start transmission.
int mbug_2151_start( mbug_device dev )
{
	return set_param( dev, PARAM_START, 0x0001 );
}


//------------------------------------------------------------------------------
// Stop transmission 
//	 force = 0: Stop gracefully after ongoing iteration
//	 force = 1: Stop instantly
int mbug_2151_stop( mbug_device dev, int force )
{
	return set_param( dev, PARAM_START, force ? 0x0100 : 0x0000  );
}

//------------------------------------------------------------------------------
// Stop transmission gracefully, i.e. after the current iteration.
int mbug_2151_stop_gracefully( mbug_device dev )
{
	return set_param( dev, PARAM_START, 0x0000 );
}

//------------------------------------------------------------------------------
// Stop transmission instantly.
int mbug_2151_stop_immediately( mbug_device dev )
{
	return set_param( dev, PARAM_START, 0x0100 );
}

//------------------------------------------------------------------------------
// Reset transmission and all parameters
int mbug_2151_reset( mbug_device dev )
{
	return set_param( dev, PARAM_RESET, 0 );
}

//------------------------------------------------------------------------------
// Get busy state. >0 if transmission is in progress
int mbug_2151_get_busy( mbug_device dev )
{
	return get_param( dev, PARAM_BUSY );
}

//------------------------------------------------------------------------------
