
#ifndef MBUG_2811_H
#define MBUG_2811_H
#ifdef __cplusplus
  extern "C" {
#endif

//------------------------------------------------------------------------------

/** MBUG Library
	2811 Thermometer device interface

	mbug_2811_list()        - Get a list of attached devices
	mbug_2811_open()		- Open a device with specified serial number
	mbug_2811_open_str()	- Open a device with specified id string
	mbug_2811_close()
	mbug_2811_set_acq_mode() - Set the acquisition mode
	mbug_2811_read()		- Read the current temeprature value
	mbug_2811_read_raw()	- Read raw sensor data
*/

//------------------------------------------------------------------------------

#include "mbug.h"

#ifndef NOT_A_TEMPERATURE
	#define NOT_A_TEMPERATURE (-274.0)
#endif

//------------------------------------------------------------------------------

/** Get a list of all available mbug-2811 devices. List is returned as an array
 *  of char containing the serial number strings. End of array is marked by a
 *  NULL pointer.
 */
const mbug_device_list  mbug_2811_list( void );

/** Open a device specified by it's serial number (as int,
 *  last digits of the serial number are matched only).
 */
mbug_device mbug_2811_open( int serial_num );

/** Open a device specified by it's id string (equals the
 *  usb serial string as returned by mbug_2820_list )
 */
mbug_device mbug_2811_open_str( const char *id );

/** Close a previously opened device.
 */
void mbug_2811_close( mbug_device dev );

/** Read the current temperature value (as Degrees Celsius)
 */
double mbug_2811_read( mbug_device dev );

/** Set the acquisition mode. See mbug.h for more details.
 */
int mbug_2811_set_acq_mode( mbug_device dev, enum mbug_acquisition_mode mode );

/** Read raw data coming directly from the sensor.
 *  Return value <0 indicates a communication arror.
 */
int mbug_2811_read_raw( mbug_device dev );

//------------------------------------------------------------------------------
#ifdef __cplusplus
  }
#endif
#endif // MBUG_2811_h