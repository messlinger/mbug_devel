
#include "mbug_2811.h"
#include "mbug_utils.h"

//-------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"MBUG-2811 command line tool                                                   \n"
"                                                                              \n"
"Usage:   mbug_2811  cmd, cmd2, ...                                            \n"
"                                                                              \n"
"    For convenience, commands have multiple abbrevations.  Commands can       \n"
"    be prepended by on or multiple '-' (unix option style),  '/' (windows     \n"
"    option style), or nothing.  Some commands take parameters, which have     \n"
"    to be separeted from the command by a ':', '=' or a whitespace.           \n"
"                                                                              \n"
"Commands:                                                                     \n"
"                                                                              \n"
"    h             Display this usage info.                                    \n"
"    help                                                                      \n"
"                                                                              \n"
"    l             List attached devices, does not read any temperature.       \n"
"    ls                                                                        \n"
"    list                                                                      \n"
"                                                                              \n"
"    r             (default) Read the current temperature measurement.         \n"
"    rd            Temperatures <= 274 (physically not possible) indicate      \n"
"    read          sensor errors.                                              \n"
"                                                                              \n"
"    d=<id>        Select device with specified serial number or id string (as \n"
"    dev=          returned by the list command). If no device is specified,   \n"
"    device=       the first available device is used.                         \n"
"                                                                              \n"
"    f=<format>    Select the data format.  Implemented formats are            \n"
"    fmt=          C, celsius : (default) Temperature in degrees Celsius,      \n"
"    format=       F, fahrenheit: Temperature in degrees Fahrenheit            \n"
"                  K, kelvin: Absolute temperature in Kelvin                   \n"
"                  R, raw: Raw sensor reading in the range 0..4095 (12 bit)    \n"
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
"    int                                                                       \n"
"    interval                                                                  \n"
"                                                                              \n"
"    n             Number of measurements to take. To take an infinite number  \n"
"    num           of measurements (default), specify 0 or \"inf\".            \n"
"    number                                                                    \n"
"                                                                              \n"
;

//--------------------------------------------------------------------------------

mbug_device thermometer = 0;

int device_serial = 0;
enum Action { Read, List, Record, Help } action = Read;
enum Format { Celsius, Fahrenheit, Kelvin, Raw } format = Celsius;

// Recording mode:
const char* rec_filename = 0;  // Destination file name
FILE* rec_file = 0;          // File handle
double rec_interval = 1.0; // Update interval
unsigned long rec_number = 0;
int rec_silent = 0;        // Silent flag (no print to stdout)

//---------------------------------------------------------------

/** Extract device serial. */
void parse_device_id( char* str )
{
	if (strncmp_upper(str,"MBUG-2811",9) == 0)
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
		else if (str_in( cmd, "f", "fmt", "format", 0 ))
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
			errorf( "#### Unknown command: %s", cmd);
	}
}


void cleanup( void )
{
	if (thermometer)
		mbug_2811_close( thermometer );
	if (rec_file) {
		fflush( rec_file );
		fclose( rec_file ); rec_file=0;
	}
}

//---------------------------------------------------------------

int main( int argc, char* argv[] )
{
	atexit( cleanup );
	parse_options( argc, argv );

	if (action==Help)
		{
			puts(usage);
			return 0;
		}

	if (action==List)  // List all attached thermometers
		{
			int i;
			mbug_device_list list = mbug_get_device_list(2811);
			for (i=0; list[i]!=0; i++ ) {
				puts(list[i]);
			}
			return 0;
		}

	// Open device
	thermometer = mbug_2811_open( device_serial );
	if (thermometer ==0 )
		errorf("#### Error opening device.");

	if (action==Read)
	{
		double tem = 0.;
		int raw = 0;
		if (format==Raw)
			raw = mbug_2811_read_raw( thermometer );
		else
			tem = mbug_2811_read( thermometer );

		if ( raw<0 || tem <= NOT_A_TEMPERATURE )
			fputs( "#### Read error\n", stdout );

		// print
		if (format==Raw)
			printf( "%d", raw );
		else if (format==Fahrenheit)
			printf( "%.2f", tem * 9./5 + 32. );
		else if (format==Kelvin)
			printf( "%.2f", tem + 273.15 );
		else // format Celsius
			printf( "%.2f", tem );
	}

	if (action==Record)
	{
		double tim = 0.0, tem = 0.0;
		int raw = 0;
		char str[500] = {0};
		unsigned long nn = 0;

		if (rec_filename != 0)
		{
			rec_file = fopen( rec_filename, "a" );
			if (rec_file == 0)
				errorf( "#### Error opening file %s\n", rec_filename );
		}

		tim = floattime();
		sprintf( str, "\n\n# %s\n# Start recording at %.3f\n# timestamp\ttemperature\n", mbug_id(thermometer), tim );
		if (rec_file)  fputs(str, rec_file);
		if (!rec_silent)  fputs(str, stdout);

		tim = floattime();
		for( nn=0; (rec_number==0)||(nn<rec_number); nn++)
		{
			tem = raw = 0;
			if (format==Raw)
				raw = mbug_2811_read_raw( thermometer );
			else
				tem = mbug_2811_read( thermometer );

			if ( raw<0 || tem <= NOT_A_TEMPERATURE )
			{
				fputs( "#### Read error\n", stdout );
				if (rec_file) fputs( "#### Read error\n", rec_file);
			}

			if (format==Raw)
				sprintf( str, "%.3f\t%d\n", tim, raw );
			else if (format==Fahrenheit)
				sprintf( str, "%.3f\t%.3f\n", tim, tem*9./5.+32. );
			else if (format==Kelvin)
				sprintf( str, "%.3f\t%.3f\n", tim, tem+273.15 );
			else // format Celsius
				sprintf( str, "%.3f\t%.3f\n", tim, tem );

			if (rec_file)  { fputs( str, rec_file ); fflush(rec_file); };
			if (!rec_silent) { fputs(str, stdout); fflush(stdout); }

			tim += rec_interval;
			waittime( tim );
		}
	}

	return 0;
}

