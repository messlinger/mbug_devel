
#ifndef MBUG_2818_H
#define MBUG_2818_H
#ifdef __cplusplus
  extern "C" {
#endif

//------------------------------------------------------------------------------

/** MBUG Library
	2818 Thermometer device interface

	mbug_2818_list()        - Get a list of attached devices
	mbug_2818_open()		- Open a device with specified serial number
	mbug_2818_open_str()	- Open a device with specified id string
	mbug_2818_close()
	mbug_2818_set_acq_mode() - Set the acquisition mode
	mbug_2818_read()		- Read a single channel
	mbug_2818_read_all()	- Read all 8 channels
	mbug_2818_read_raw()	- Read raw binary values
	mbug_2818_set_tris_bits() - Set channels I/O direction bits
	mbug_2818_set_dout_bits() - Set channels digital output bits
	mbug_2818_enable_pwm()  - Enable/disable PWM generation on channel 1
	mbug_2818_set_pwm()     - Set PWM uty cycle for channel 1
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
mbug_device mbug_2818_open( unsigned long serial_num );

/** Open a device specified by it's id string (equals the
 *  usb serial string as returned by mbug_2820_list )
 */
mbug_device mbug_2818_open_str( const char *id );

/** Close a previously opened device.
 */
void mbug_2818_close( mbug_device dev );

/** Set the acquisition mode (default: instantaneous).
 */
int mbug_2818_set_acq_mode( mbug_device dev, enum mbug_acquisition_mode mode );

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

/** Set the channels I/O direction (tristate, 0: Digital output, 1:Analog input),
    bits packed in one integer
 */
int mbug_2818_set_tris_bits( mbug_device dev, unsigned short tris_bits );

/** Set the channels digital output state (0: low, 1:high),
    bits packed in one integer
 */
int mbug_2818_set_dout_bits( mbug_device dev, unsigned short dout_bits );

/** Enable PWM generation on channel 1 (0: off, 1:on).
    Argument chan is only supplied for upward compatibility and is ignored.
    The user must also explicitely configure channel 1 as digital output.
 */
int mbug_2818_enable_pwm( mbug_device dev, unsigned char chan, int enable );

/* Set the PWM duty cycle for channel 1 to duty/1024.
   Argument chan is only supplied for upward compatibility and is ignored.
 */
int mbug_2818_set_pwm( mbug_device dev, unsigned char chan, unsigned short duty );


//------------------------------------------------------------------------------
#ifdef __cplusplus
  }
#endif
#endif // MBUG_2818_h
