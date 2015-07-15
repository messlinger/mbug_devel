
#ifndef MBUG_2820_H
#define MBUG_2820_H
#ifdef __cplusplus
  extern "C" {
#endif

//------------------------------------------------------------------------------

/** MBUG Library
	2820 Environmeter device interface

	mbug_2820_list()              - Get a list of attached devices
	mbug_2820_open()		      - Open a device with specified serial number
	mbug_2820_open_str()	      - Open a device with specified id string
	mbug_2820_close()             - Close device
	mbug_2820_set_acq_mode()      - Set acquisition mode
	mbug_2820_read_temperature()  - Read temperature
	mbug_2820_read_humidity()     - Read humidity
	mbug_2820_read()		      - Read temperature and humidity
	mbug_2820_read_raw()	      - Read raw binary values
*/

//------------------------------------------------------------------------------

#include "mbug.h"

#ifndef NOT_A_TEMPERATURE
	#define NOT_A_TEMPERATURE (-274.0)
#endif

//------------------------------------------------------------------------------

/** Get a list of all available mbug_2820 devices. List is returned as an array
 *  of char containing the serial number strings. End of array is marked by a
 *  NULL pointer.
 */
const mbug_device_list  mbug_2820_list( void );

/** Open a device specified by it's serial number (as int,
 *  last digits of the serial number are matched only).
 */
mbug_device mbug_2820_open( unsigned long serial_num );

/** Open a device specified by it's id string (equals the
 *  usb serial string as returned by mbug_2820_list )
 */
mbug_device mbug_2820_open_str( const char *id );

/** Close a previously opened device.
 */
void mbug_2820_close( mbug_device dev );

/** Set the acquisition mode (default: instantaneous).
 *  see mbug.h for further explanations.
 */
int mbug_2820_set_acq_mode( mbug_device dev, enum mbug_acquisition_mode mode );

/** Read measurements: Temperature, humidity. Data is written to the passed
 *  pointer locations. Values < NOT_A_TEMPERATURE indicate sensor errors.
 *  Pass NULL if you are not interested in the respective value.
 */
int mbug_2820_read( mbug_device dev, double *temperature, double *humidity );

/** Read temperature. NOTE: Correpsonding humidty measurement is DISCARDED.
 */
double mbug_2820_read_temperature( mbug_device dev );

/** Read humidity. NOTE: Correpsonding temperature measurement is DISCARDED.
 */
double mbug_2820_read_humidity( mbug_device dev );

/** Read raw measurement data (as directly coming from the sensors).
 *  Data is returned as a single 32 bit integer. Return value 0xFFFFFFFF
 *  indicates a sensor error.
 */
long mbug_2820_read_raw( mbug_device dev );

//------------------------------------------------------------------------------
#ifdef __cplusplus
  }
#endif
#endif // MBUG_2820_h
