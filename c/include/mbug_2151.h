
#ifndef MBUG_2151_H
#define MBUG_2151_H
#ifdef __cplusplus
  extern "C" {
#endif

//------------------------------------------------------------------------------

/** MBUG Library
	MBUG-2155 IR Transmitter

       Stephan Messlinger
       Last change: 2013-08-04

	mbug_2151_list()                    - Get a list of attached devices
	mbug_2151_open()					- Open a device with specified serial number
	mbug_2151_open_str()				- Open a device with specified id string
	mbug_2151_close()
	mbug_2151_set_sequence()            - Set transmission sequence (selectable transmission mode)
	mbug_2151_set_seq_bitstream()		- Set bitstream transmission sequence (max 255 bytes)
	mbug_2151_set_seq_times_8bit()		- Set 8-bit timed transmission sequence (max 255 bytes)
	mbug_2151_set_seq_times_16bit()		- Set 16-bit timed transmission sequence (max 127 words)
	mbug_2151_get_seq_length()			- Get the length of the transmission sequence (number of bytes)
	mbug_2151_set_iterations()			- Set number of transmission sequence iterations
	mbug_2151_get_iterations()			- Get number of transmission sequence iterations
	mbug_2151_get_base_clock()			- Get the devices base bit clock frequency in Hz
	mbug_2151_set_clock_div()			- Set the clock divider (clock = base_clock/div)
	mbug_2151_get_clock_div()			- Get the clock divider
    mbug_2151_set_bitrate()				- Set bit rate (via clock div)
	mbug_2151_start()					- Start transmission
	mbug_2151_stop_gracefully()			- Stop transmission gracefully, i.e. after the current iteration.
	mbug_2151_stop_immediately()		- Stop transmission immediately
	mbug_2151_reset()					- Reset transmission and all parameters
    mbug_2151_get_busy()				- Get busy state. True if transmission is in progress
*/

//------------------------------------------------------------------------------

#include "mbug.h"

// Transmission modes (use in set_sequence)
typedef  enum mbug_transmission_mode  mbug_2151_tx_mode;

//------------------------------------------------------------------------------

/** Get a list of all available mbug-2151 devices. List is returned as an array
 *  of char containing the serial number strings. End of array is marked by a
 *  NULL pointer.
 */
const mbug_device_list  mbug_2151_list( void );

/** Open a device specified by it's serial number (as int,
 *  last digits of the serial number are matched only).
 */
mbug_device mbug_2151_open( int serial_num );

/** Open a device specified by it's id string (equals the
 *  usb serial string as returned by mbug_2820_list )
 */
mbug_device mbug_2151_open_str( const char *id );

/** Close a previously opened device.
 */
void mbug_2151_close( mbug_device dev );

/** Set transmission sequence (max 255 bytes length) with selectable transmission mode.
 */
int mbug_2151_set_sequence( mbug_device dev, unsigned char* data, size_t nbytes, mbug_2151_tx_mode tx_mode );

/** Set transmission sequence in bitstream mode (max 255 bytes length)
 */
int mbug_2151_set_seq_bitstream( mbug_device dev, unsigned char* data, size_t nbytes );

/** Set transmission sequence in timed mode (8-bit resolution, max 255 bytes length)
 */
int mbug_2151_set_seq_times_8bit( mbug_device dev, unsigned char* data, size_t nbytes );

/** Set transmission sequence in timed mode (16-bit resolution, max 127 bytes length)
 */
int mbug_2151_set_seq_times_16bit( mbug_device dev, unsigned short* data, size_t nbytes );

/** Get the length of the currently stored transmission sequence in bytes
 */
int mbug_2151_get_seq_length( mbug_device dev );

/** Set number of transmission sequence iterations
0: infinite, Maximum: 2**16-1
 */
int mbug_2151_set_iterations( mbug_device dev, unsigned int niter );

/** Get the currently set number of iterations
 */
int mbug_2151_get_iterations( mbug_device dev );

/** Get the devices base clock frequency in Hz
 */
int mbug_2151_get_base_clock( mbug_device dev );

/** Set clock divider. => Bitrate = base_clock/div
Minimum div is 10 for stream mode, maximum div is 2**16-1.
 */
int mbug_2151_set_clock_div( mbug_device dev, unsigned int div );

/** Get the currently set clock divider.
 */
int mbug_2151_get_clock_div( mbug_device dev );

/** Set bit rate (via clock div). Returns the actually set value.
 */
double mbug_2151_set_bitrate( mbug_device dev, double freq );

/** Set the timebase (via clock div). Returns the actually set value.
 */
double mbug_2151_set_timebase( mbug_device dev, double interval );

/** Start transmission.
 */
int mbug_2151_start( mbug_device dev );

/** Stop transmission.
 force = 0: Stop gracefully after ongoing iteration
 force = 1: Stop instantly
 */
int mbug_2151_stop( mbug_device dev, int force );

/** Stop transmission gracefully, i.e. after the current iteration.
 */
int mbug_2151_stop_gracefully( mbug_device dev );

/** Stop transmission instantly.
 */
int mbug_2151_stop_immediately( mbug_device dev );

/** Reset transmission and all parameters
 */
int mbug_2151_reset( mbug_device dev );

/** Get busy state. True if transmission is in progress
 */
int mbug_2151_get_busy( mbug_device dev );

//------------------------------------------------------------------------------
#ifdef __cplusplus
  }
#endif
#endif // MBUG_2151_H