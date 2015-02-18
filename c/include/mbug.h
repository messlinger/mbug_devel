
#ifndef MBUG_H
#define MBUG_H
#ifdef __cplusplus
  extern "C" {
#endif

//------------------------------------------------------------------------------

/** MBUG Library

  Very thin layer above libusb-0 to provide a slightly more convenient interface.

    mbug_get_device_list()  -  Get a list of available devices.
    mbug_open()             -  Open a device with specified type and serial.
    mbug_close()
    mbug_read()             -  Raw data communication
    mbug_write()
	mbug_error()            -  Get last error message (plain text).
*/

//------------------------------------------------------------------------------

typedef const char * const *  mbug_device_list;
struct mbug_device_struct;
typedef struct mbug_device_struct mbug_device_struct;
typedef mbug_device_struct* mbug_device;

//------------------------------------------------------------------------------
/** Some parameter definitions that are used in more than one device.
 */

enum mbug_acquistion_mode {  // (Most devices only support a subset of acquisition modes)
	ACQ_MODE_INST = 0, // Instantaneous: Use last measured value if not read before, else wait for next.
	ACQ_MODE_LAST,     // Last: Always use last valid value, never block.
	ACQ_MODE_NEXT,     // Next: Always wait for next incoming value.
	ACQ_MODE_TRIG,	   // Triggered: Start acquisition by software trigger
	ACQ_MODE_HWTRIG    // Hardware triggered: Start acquisition by hardware trigger
};

enum mbug_transmission_mode {
	TX_MODE_BITSTREAM = 0,	// Bitstream mode: Transmit sequence bits subsequently with
						    //    fixed bitrate. Start with the LSB of the first sequence byte.
	TX_MODE_TIMED_8, 	    // Timed 8-bit mode: Sequence bytes specifiy the interval
					        //    times (high/low alternatingly).
	TX_MODE_TIMED_16	    // Timed 8-bit mode: Like 8-bit mode, but 2 consecutive bytes
};

//------------------------------------------------------------------------------

/** Get a list of all available devices which match given device_type.
 *  Pass 0 to list all devices. List is returned as an array of char *
 *  containing the serial number strings. End of array is marked by a
 *  NULL pointer.
 */
const mbug_device_list  mbug_get_device_list( unsigned int device_type );

/** Open a device specified by it's device type and serial number
 *  (as int, last digits of the serial number are matched only).
 */
mbug_device mbug_open_int( unsigned int device_type, unsigned long serial );

/** Open a device specified by it's id string (equals the USB serial number,
 * as returned by mbug_device_list: "MBUG-TTTT-SSSSSS"). The leading "MBUG-"
 * may be omitted.
 */
mbug_device mbug_open_str( const char* id );

/** Extract device type from id string. Returns < 0 for invalid ids.
 */
int mbug_type_from_id( const char *id );

/** Extract serial number from id string. Returns < 0 for invalid ids.
 */
long mbug_serial_from_id( const char *id );

/** Close a previously opened device.
 */
void mbug_close( mbug_device dev );

/** Read data from device (blocking).
 */
int mbug_read( mbug_device dev, void* data, int size );

/** Write data to device (blocking).
 */
int mbug_write( mbug_device dev, const void* data, int size );

/** Get id string from opened device handle ("MBUG-XXXX-XXXXXX").
 */
const char* mbug_id( mbug_device dev );

/** Get last error message (in plain text).
 */
const char* mbug_error( mbug_device dev );

//------------------------------------------------------------------------------

#ifdef __cplusplus
  }
#endif
#endif //MBUG_H
