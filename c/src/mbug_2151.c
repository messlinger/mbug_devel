
//==============================================================================
/* MBUG-2151 interface implementation

		Mostly identical to mbug_2165.c, but without carrier modulation.
		Any changes in this file should also be applied in mbug_2165.c.
*/
//==============================================================================

#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // toupper()
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
// Set transmission sequence (max 255 bytes, selectable transmission mode)
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
// Set transmission sequence in bitstream mode (max 255 bytes)
int mbug_2151_set_seq_bitstream( mbug_device dev, unsigned char* data, size_t nbytes )
{
	return mbug_2151_set_sequence( dev, data, nbytes, TX_MODE_BITSTREAM );
}

//------------------------------------------------------------------------------
// Set transmission sequence in 8-bit time mode (max 255 bytes)
int mbug_2151_set_seq_times_8bit( mbug_device dev, unsigned char* data, size_t nbytes )
{
	return mbug_2151_set_sequence( dev, data, nbytes, TX_MODE_TIMED_8 );
}

//------------------------------------------------------------------------------
// Set transmission sequence in 16-bit time mode (max 127 items)
int mbug_2151_set_seq_times_16bit( mbug_device dev, unsigned short* data, size_t nitems )
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
// Set the transmission time base (in seconds, via clock div).
// Minimum timebase is 2.5us for bitstream mode, 0.25us for timed modes.
// Maximum timebase is 16.38375ms.
// Resolution is 0.25us.
// Returns the actually set timebase or <0 in case of an error.
double mbug_2151_set_timebase( mbug_device dev, double interval )
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

int mbug_2151_ab440s_str_to_addr( const char *str ) {
	int addr = 0;
	int i = 0;

	for(i = 0; str[i]; i++) {
		if(str[i] >= '1' && str[i] <= '5') {
			addr |= 1<<(9-str[i]+'1');
		} else if(toupper(str[i]) >= 'A' && toupper(str[i]) <= 'E') {
			addr |= 1<<(4-toupper(str[i])+'A');
		}
	}

	return addr;
}

int mbug_2151_ab440s_switch_addr( mbug_device dev, int addr, int state ) {
	unsigned char seq[16];
	int i;

	if(mbug_2151_set_bitrate(dev, 3125) == -1) {
		return -1;
	}

	if(mbug_2151_set_iterations(dev, 3) == -1) {
		return -1;
	}

	for(i = 0; i < 12; i++) {
		seq[i] = 0x71;
	}

	if(state == 1) {
		seq[10] = 0x11;
	} else {
		seq[11] = 0x11;
	}
	seq[12] = 1;
	seq[13] = seq[14] = seq[15] = 0;
	for(i = 0; i < 10; i++) {
		seq[9-i] = (addr & (1<<i)) ? 0x11 : 0x71;
	}
	if(mbug_2151_set_sequence(dev, seq, 16, TX_MODE_BITSTREAM) == -1) {
		return -1;
	}
	return mbug_2151_start(dev);
}

int mbug_2151_ab440s_switch_str( mbug_device dev, const char *addr, int state ) {
	return mbug_2151_ab440s_switch_addr(dev, 
										mbug_2151_ab440s_str_to_addr(addr),
										state);
}

int mbug_2151_dmv7008_cmd_addr(mbug_device dev, int syscode, int channel, int cmd) {
	int i;
	char seq[59];
	int bits;
	const unsigned char cmdbits[5][4] = {
		{ 0xE1, 0x70, 0xEB, 0x7A },
		{ 0x00, 0x11, 0x0A, 0x1B },
		{ 0x82, 0x93, 0x88, 0x99 },
		{ 0x41, 0x50, 0x4B, 0x5A },
		{ 0xC3, 0xD2, 0xC9, 0xD8 }
	};

	syscode = syscode & 0xFFF;
	if(channel < 0 || channel > 4) {
		return -1;
	}

	bits = (1 << 20) | (syscode << 8) | (cmdbits[channel][cmd] << 0);

	for(i = 0; i < 21; i++) {
		seq[i] = ((bits >> (20-i)) & 1) ? 0xC0 : 0xFC;
	}
	for( ; i < 59; i++) {
		seq[i] = 0;
	}

	if(mbug_2151_set_bitrate(dev, 4167) == -1) {
		return -1;
	}

	if(mbug_2151_set_iterations(dev, 4) == -1) {
		return -1;
	}

	if(mbug_2151_set_sequence(dev, seq, 59, TX_MODE_BITSTREAM) == -1) {
		return -1;
	}

	return mbug_2151_start(dev);
}

int mbug_2151_dmv7008_cmd_str(mbug_device dev, const char *addr, int cmd) {
	static int syscode = 0;
	int channel;

	char *colon = strchr(addr, ':');
	if(colon != NULL) {
		syscode = atoi(addr);
		channel = atoi(colon+1);
	} else {
		channel = atoi(addr);
	}

	return mbug_2151_dmv7008_cmd_addr(dev, syscode, channel, cmd);
}

int mbug_2151_rs200_cmd_addr(mbug_device dev, int syscode, int channel, int cmd) {
	int i;
	unsigned short seq[26*2];
	unsigned int data;

	if(syscode < 0 || syscode > 255) {
		return -1;
	}

	if(channel < 0 || channel == 4 || channel > 5) {
		return -1;
	}

	data = (channel << 4) | (cmd << 7) | ((1^cmd) << 2);
	if(channel == 1 || channel == 2) {
		data ^= 1<<7;
	}
	data = (0x19 << 20)
		| (syscode << 12)
		| (data << 4)
		| (0x9 + syscode + (syscode>>4) + data + (data>>4)) & 0x0F;

	for(i = 0; i < 26; i++) {
		if(data & (1<<(25-i))) {
			seq[2*i+0] = 600;
			seq[2*i+1] = 3400;
		} else {
			seq[2*i+0] = 1300;
			seq[2*i+1] = 3400;
		}
	}
	seq[26*2-1] += 30000;
	
	if(mbug_2151_set_timebase(dev, 1e-6) == -1) {
		return -1;
	}

	if(mbug_2151_set_iterations(dev, 4) == -1) {
		return -1;
	}

	if(mbug_2151_set_seq_times_16bit(dev, seq, 26*2) == -1) {
		return -1;
	}

	return mbug_2151_start(dev);
}

int mbug_2151_rs200_cmd_str(mbug_device dev, char *addr, int cmd) {
	static int syscode = 0;
	int channel;

	char *colon = strchr(addr, ':');
	char *chanptr;
	if(colon != NULL) {
		syscode = atoi(addr);
		chanptr = colon+1;
	} else {
		chanptr = addr;
	}

	if(toupper(*chanptr) == 'A'
	   || toupper(*chanptr) == 'M') {
		channel = 5;
	} else {
		channel = atoi(chanptr) - 1;
	}

	return mbug_2151_rs200_cmd_addr(dev, syscode, channel, cmd);
}
