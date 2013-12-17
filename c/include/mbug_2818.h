
#ifndef MBUG_2818_H
#define MBUG_2818_H
#ifdef __cplusplus
  extern "C" {
#endif 

//------------------------------------------------------------------------------

/** MBUG Library
	2818 Thermometer device interface

     Stephan Messlinger
     Last change: 2011-10-28

	mbug_2818_list()        - Get a list of attached devices
	mbug_2818_open()		- Open a device with specified serial number
	mbug_2818_open_str()	- Open a device with specified id string
	mbug_2818_close()
	mbug_2818_read()		- Read a single channel
	mbug_2818_read_all()	- Read all 8 channels
	mbug_2818_read_raw()	- Read raw binary values
*/

//------------------------------------------------------------------------------

#include "mbug.h"

#ifndef NOT_A_TEMPERATURE
	#define NOT_A_TEMPERATURE (-274.0)
#endif
//------------------------------------------------------------------------------

/** Get a list of all available mbug-2818 devices. List is returned as an array
 *  of char containing the serial number strings. End of array is marked by a
 *  NULL pointer.
 */
const mbug_device_list  mbug_2820_list( void );

/** Open a device specified by it's serial number (as int, 
 *  last digits of the serial number are matched only).
 */
mbug_device mbug_2818_open( int serial_num );

/** Open a device specified by it's id string (equals the 
 *  usb serial string as returned by mbug_2820_list )
 */
mbug_device mbug_2818_open_str( const char *id );

/** Close a previously opened device.
 */
void mbug_2818_close( mbug_device dev );

/** Read one temperature channel (as Degrees Celsius).
	Channel = 0..7
 */
double mbug_2818_read( mbug_device dev, int channel );

/** Read all temperature values (as Degrees Celsius).
    Data is written to the passed double buffer, n specifies 
	the number of channels to read (max 8).
 */
int mbug_2818_read_all( mbug_device dev, double temperatures[], int n );

/** Read raw 16-bit data values (as coming from ADC).
    Data is written to the passed buffer buffer, n specifies 
	the number of channels to read (max 8).
 */
int mbug_2818_read_raw( mbug_device dev, unsigned short data[], int n );

//------------------------------------------------------------------------------
#ifdef __cplusplus
  }
#endif 
#endif // MBUG_2818_h
