
#ifndef MBUG_2110_H
#define MBUG_2110_H
#ifdef __cplusplus
  extern "C" {
#endif

//------------------------------------------------------------------------------

/** MBUG Library
	2110 GPIO Interface

	mbug_2110_list()        - Get a list of attached devices
	mbug_2110_open()		- Open a device with specified serial number
	mbug_2110_open_str()	- Open a device with specified id string
	mbug_2110_close()
	mbug_2110_get_gpio()	- Read the current GPIO input states
	mbug_2110_set_gpio()	- Set the GPIO output states
	mbug_2110_set_tris()	- Set the gpio tristate directions
	mbug_2110_enable_adc()	- Enable/disable the A/D converter input channels on GPIO 3-10
	mbug_2110_read_adc()	- Read the A/D converter input voltages
	mbug_2110_enable_pwm()	- Enable the pwm output on GPIO 0.
	mbug_2110_set_pwm()		- Set the pwm duty cycle
*/

//------------------------------------------------------------------------------

#include "mbug.h"

//------------------------------------------------------------------------------


/** Get a list of all available mbug-2151 devices. List is returned as an array
 *  of char containing the serial number strings. End of array is marked by a
 *  NULL pointer.
 */
const mbug_device_list  mbug_2151_list( void );

/** Open a device specified by it's serial number (as int,
 *  last digits of the serial number are matched only).
 */
mbug_device mbug_2110_open( int serial_num );

/** Open a device specified by it's id string (equals the
 *  usb serial string as returned by mbug_2820_list )
 */
mbug_device mbug_2110_open_str( const char *id );

/** Close a previously opened device.
 */
void mbug_2110_close( mbug_device dev );

/** Read the current GPIO input states (12 bits, little endian bit packed).
GPIO lines that are not configured for digital input will give undefined results.
 */
int mbug_2110_get_gpio( mbug_device dev );

/** Set the GPIO output states (12 bits, little endian bit packed). States
will only be applied to GPIO ports that are configured for digital output.
 */
int mbug_2110_set_gpio( mbug_device dev, unsigned int bits );

/** Set the GPIO tristate directions (12 bits, little endian bit packed,
0: Output, 1: Input).
 */
int mbug_2110_set_tris( mbug_device dev, unsigned int tris );

/** Enable the analog input channels (8 bits, little endian bit packed, 1:enable,
0:disable). ADC channels 0..7 are multiplexed with GPIO channels 3..10.
 */
int mbug_2110_enable_adc( mbug_device dev, unsigned int channels );

/** Read all analog input channels (wether enabled or not). ADC resolution is
10 bit (0-1023). Readings are returned as array of 8*16 bit integers.
 */
const unsigned short * mbug_2110_read_adc( mbug_device dev );

/** Enable/disable the pwm output on GPIO 0.
 */
int mbug_2110_enable_pwm( mbug_device dev, int channels );

/** Set the pwm duty cycle. Duty cycle resolution is 10 bit. For convenience,
the duty cycle can be passed as integer in range 0..1023 or as float in the
range 0..1.
 */
int mbug_2110_set_pwm( mbug_device dev, int duty );


//------------------------------------------------------------------------------
#ifdef __cplusplus
  }
#endif
#endif // MBUG_2110_h