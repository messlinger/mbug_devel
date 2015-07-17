
#include "mbug_2820.h"
#include "mbug_utils.h"
#include <stdio.h>

//-------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"MBUG-2820 command line tool                                                   \n"
"                                                                              \n"
"Usage:   mbug_2820  cmd, cmd2, ...                                            \n"
"                                                                              \n"
"    For convenience, commands have multiple abbrevations.  Commands can       \n"
"    be prepended by on or multiple '-' (unix option style),  '/' (windows     \n"
"    option style), or nothing.  Some commands take parameters, which have     \n"
"    to be separeted from the command by a ':', '=' or a whitespace.           \n"
"                                                                              \n"
"    The data output is in degree Celsius and percent relative humidity,       \n"
"    comma separated in this order. Other data formats may be implemented in   \n"
"    future versions. Time stamps are stored in standard POSIX format          \n"
"    (i.e. seconds since UTC 1970-01-01T00:00:00Z).                            \n"
"                                                                              \n"
"Commands:                                                                     \n"
"                                                                              \n"
"    h             Display this usage info.                                    \n"
"    help                                                                      \n"
"                                                                              \n"
"    l             List attached devices.                                      \n"
"    ls                                                                        \n"
"    list                                                                      \n"
"                                                                              \n"
"    r             (default) Read the current temperature and humidity values. \n"
"    rd            Values <= 274 (physically not possible) indicate sensor     \n"
"    read          errors. The values are returned comma separated:            \n"
"                  <temperature>,<humidty>                                     \n"
"                                                                              \n"
"    d=<id>        Select device with specified serial number or id string     \n"
"    dev=          as returned by the list command). If no device is           \n"
"    device=       specified, the first available device is used.              \n"
"                                                                              \n"
"    fmt=<format>  Select the data format.  Implemented formats are            \n"
"    format=       C, celsius : (default) Temperature in degrees Celsius,      \n"
"                  F, fahrenheit: Temperature in degrees Fahrenheit            \n"
"                  K, kelvin: Absolute temperature in Kelvin                   \n"
"                  R, raw: Raw sensor reading (custom format)                  \n"
"                                                                              \n"
"    log           Recording mode: Take periodic measurements with a specified \n"
"    rec           interval. The data is printed to stdout. If a file is       \n"
"    record        specified, the data is also printed to the specified file.  \n"
"                                                                              \n"
"    file          Output filename used in recording mode                      \n"
"                                                                              \n"
"    silent        Suppress data output to stdout, write to file only.         \n"
"                                                                              \n"
"    i             Measurement interval in seconds. Default is 1.              \n"
"    int           For maximum rate (0.1), set to 0 or \"min\".                \n"
"    interval      For small intervals, the timing may be inaccurate.          \n"
"                                                                              \n"
"    n             Number of measurements to take. To take an infinite number  \n"
"    num           of measurements (default), specify 0 or \"inf\".            \n"
"    number        Use Ctrl+C to terminate an ongoing measurement.             \n"
"                                                                              \n"
;

//--------------------------------------------------------------------------------

mbug_device device = 0;  // Device handle

int device_serial = 0;
enum Action { Read, List, Record, Help } action = Read;
enum Format { Celsius, Fahrenheit, Kelvin, Raw } format = Celsius;

// Measurement results
long raw = 0;
double temperature = 0, humidity = 0;

// Recording mode:
const char* rec_filename = 0;  // Destination file name
FILE* rec_file = 0;          // File handle
double rec_interval = 1.0; // Update interval
unsigned long rec_number = 0;
int rec_silent = 0;        // Silent flag (no print to stdout)

//---------------------------------------------------------------


/** Extract device id. */
void parse_device_id( char* str )
{
	if (strncmp_upper(str,"MBUG-2810",9) == 0)
		device_serial = mbug_serial_from_id(str);
	else device_serial = str_to_uint(str);
	if (device_serial < 0) errorf( "#### Invalid device id: %s", str );
}

/** Extract temperature format. */
void parse_format( char* str )
{
	char* s = str;
	if (s == 0) errorf( "#### Invalid format: %s", str );
	while (*s != 0) *s++ = toupper(*s);
	if (str_in(str, "C", "CELSIUS", 0 ))  format = Celsius;
	else if (str_in(str, "F", "FAHRENHEIT", 0 ))  format = Fahrenheit;
	else if (str_in(str, "K", "KELVIN", 0 ))  format = Kelvin;
	else if (str_in(str, "R", "RAW", 0 ))  format = Raw;
	else errorf( "#### Invalid format: %s", str );
}


/** Extract record output filename. */
void parse_recfilename( const char* str )
{
	if (str==0 || *str=='\0')
		errorf( "#### Invalid filename: %s", str );
	rec_filename = str;
}

/** Extract record interval. */
parse_recinterval( const char* str )
{
	rec_interval = str_to_float(str);
	if (rec_interval<0)
		errorf( "#### Invalid interval: %s", str );
}

/** Extract number of records. */
parse_recnumber( const char* str )
{
	if (strcmp_upper(str,"INF") == 0)
		rec_number = 0;
	else if ((rec_number = str_to_uint(str))<0)
		errorf( "#### Invalid number: %s", str );
}

/** Parse the argv for command line parameters. */
void parse_options( int argc, char* argv[] )
{
	char *cmd;
	arg_tok( argv+1, 0 );
	while (cmd = arg_tok(0,":="))
	{
		cmd += strspn( cmd, "-/" );
		if (str_in( cmd, "h", "help", 0 ))
			action = Help;
		else if (str_in( cmd, "l", "ls", "list", 0 ))
			action = List;
		else if (str_in( cmd, "r", "rd", "read", 0 ))
			action = Read;
		else if (str_in( cmd, "d", "dev", "device", 0 ))
			parse_device_id( arg_tok(0,"") );
		else if (str_in( cmd, "fmt", "format", 0 ))
			parse_format( arg_tok(0,"") );
		else if (str_in( cmd, "log", "rec", "record", 0 ))
			action = Record;
		else if (str_in( cmd, "file", 0 ))
			parse_recfilename( arg_tok(0,"") );
		else if (str_in( cmd, "s", "silent", 0 ))
			rec_silent = 1;
		else if (str_in( cmd, "i", "int", "interval", 0 ))
			parse_recinterval( arg_tok(0,"") );
		else if (str_in( cmd, "n", "num", "number", 0 ))
			parse_recnumber( arg_tok(0,"") );
		else
			errorf( "#### Unknown command: %s", cmd );
	}
}


void cleanup( void )
{
	if (device)
		mbug_2820_close( device );
	if (rec_file) {
		fflush( rec_file );
		fclose( rec_file ); rec_file=0;
	}
}

int read_data(void)
{
	int ret = 0;
	temperature = humidity = raw = 0;
	if (format==Raw)
		raw = mbug_2820_read_raw( device );
	else
		ret = mbug_2820_read( device, &temperature, &humidity);

	if ( raw<0 || temperature <= NOT_A_TEMPERATURE ||
		 ret<0 || humidity <= NOT_A_TEMPERATURE )
		return -1;

	if (format == Fahrenheit)
		temperature = temperature * 9./5 + 32. ;
	else if (format == Kelvin)
		temperature += 273.15 ;
	return 0;
}

//---------------------------------------------------------------

int main( int argc, char* argv[] )
{
	atexit( cleanup );
	parse_options( argc, argv );

	if (action==List)
	{
		// List all attached devices
		int i;
		mbug_device_list list = mbug_get_device_list(2820);
		for (i=0; list[i]!=0; i++ ) {
			puts(list[i]);
		}
		return 0;
	}

	if (action==Help)
	{
		puts(usage);
		return 0;
	}

	// Open device
	device = mbug_2820_open( device_serial );
	if (device ==0 )
		errorf("#### Error opening device.");

	// Read once
	if (action==Read)
	{
		int err = 0;
		err = mbug_2820_set_acq_mode( device, ACQ_MODE_INST );
		if (err<0) errorf( "#### Error setting acquisition mode\n" );

		err = read_data();
		if (err) errorf( "#### Read error\n" );

		if (format==Raw)
			printf( "%d", raw );
		else // format already converted by read_temperature()
			printf( "%.3f,%.2f\n", temperature, humidity );
	}

	if (action==Record)
	{
		int err = 0;
		double tim = 0.0;
		char sout[500] = {0};
		unsigned long nn = 0;

		if (rec_filename != 0)
		{
			rec_file = fopen( rec_filename, "a" );
			if (rec_file == 0)
				errorf( "#### Error opening file %s\n", rec_filename );
		}

		// Acquisition mode: For slow acquisitions, use synchronous mode.
		if (rec_interval > 0.15) {
			err = mbug_2820_set_acq_mode( device, ACQ_MODE_SYNC );
			if (err)  // Pre 2015-07 firmware versions do not support synchronous acquisition
				err = mbug_2820_set_acq_mode( device, ACQ_MODE_INST );
		}
		else  //   For high acquisitions rate, use instantaneous mode (continuous background acquisition)
			err = mbug_2820_set_acq_mode( device, ACQ_MODE_INST );
		if (err<0) fputs( "#### Error setting acquisition mode\n", stdout );

		// File header
		tim = floattime();
		sprintf( sout, "\n\n# %s\n# Start recording at %.2f (%s)\n", mbug_id(device), tim, strtime(tim) );
		if (format==Raw) 
			 sprintf( sout, "# timestamp\tdata\n" );
		else sprintf( sout, "# timestamp\ttemp\thumidity\n" );
		if (rec_file)  fputs(sout, rec_file);
		if (!rec_silent)  fputs(sout, stdout);

		// Measurement loop
		tim = floattime();
		for( nn=0; (rec_number==0)||(nn<rec_number); nn++)
		{
			double mtim = 0;
			err = read_data();
			mtim = floattime();
			if (err<0) {
				if (rec_file) fputs( "#### Read error\n", rec_file);
				errorf( "#### Read error\n" );
			}

			if (format==Raw)
				sprintf( sout, "%.2f\t0x%X\n", mtim, raw );
			else // formats already converted by read_temperature()
				sprintf( sout, "%.2f\t%.3f\t%.2f\n", mtim, temperature, humidity );

			permit_abort(0); // Prevent abortion during file write to prevent data corruption
			if (rec_file)  { fputs( sout, rec_file ); fflush(rec_file); };
			if (!rec_silent) { fputs(sout, stdout); fflush(stdout); }
			permit_abort(1);

			tim += rec_interval;
			waittime( tim );
		}
	}

	return 0;
}

