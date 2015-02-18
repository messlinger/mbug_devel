
#ifndef MBUG_2810_H
#define MBUG_2810_H
#ifdef __cplusplus
  extern "C" {
#endif

//------------------------------------------------------------------------------

/** MBUG Library
	2810 Thermometer device interface

	mbug_2810_list()        - Get a list of attached devices
	mbug_2810_open()		- Open a device with specified serial number
	mbug_2818_open_str()	- Open a device with specified id string
	mbug_2810_close()
	mbug_2810_read()		- Read the current temeprature value
	mbug_2810_set_mode()	- Set the acquisition mode

*/

//------------------------------------------------------------------------------

#include "mbug.h"

#ifndef NOT_A_TEMPERATURE
	#define NOT_A_TEMPERATURE (-274.0)
#endif

typedef  enum mbug_acquistion_mode  mbug_2810_mode;

//------------------------------------------------------------------------------

/** Get a list of all available mbug-2810 devices. List is returned as an array
 *  of char containing the serial number strings. End of array is marked by a
 *  NULL pointer.
 */
const mbug_device_list  mbug_2810_list( void );

/** Open a device specified by it's serial number (as int,
 *  last digits of the serial number are matched only).
 */
mbug_device mbug_2810_open( int serial_num );

/** Open a device specified by it's id string (equals the
 *  usb serial string as returned by mbug_2820_list )
 */
mbug_device mbug_2810_open_str( const char *id );

/** Close a previously opened device.
 */
void mbug_2810_close( mbug_device dev );

/** Read the current temperature value (as Degrees Celsius)
 */
double mbug_2810_read( mbug_device dev );

/** Set the acquisition mode:
  	MODE_INST: Use last measured value if not read before, else wait for next.
	MODE_LAST: Always use last valid value, never block.
	MODE_NEXT: Always wait for next incoming value.
 */
int mbug_2810_set_mode( mbug_device dev, mbug_2810_mode mode );

/** Read raw data coming directly from the sensor.
 *  Return value <0 indicates a communication arror.
 */
int mbug_2810_read_raw( mbug_device dev );

//------------------------------------------------------------------------------
#ifdef __cplusplus
  }
#endif
#endif // MBUG_2810_h