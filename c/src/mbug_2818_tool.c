
#include "mbug_2818.h"
#include "mbug_utils.h"

//-------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"MBUG-2818 command line tool                                                   \n"
"                                                                              \n"
"Usage:   mbug_2818  cmd, cmd2, ...                                            \n"
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
"    r             (default) Read the current temperature values.              \n"
"    rd            Temperatures <= -274 (physically not possible)              \n"
"    read          indicate sensor errors.                                     \n"
"                                                                              \n"
"    d=<id>        Select device with specified serial number or id string (as \n"
"    dev=          returned by the list command). If no device is specified,   \n"
"    device=       the first available device is used.                         \n"
"                                                                              \n"
"    c=<channel>   Select a specific channel, or a comma seperated list of     \n"
"    ch=           channels (_no_ whitespaces) to be printed (comma separated).\n"
"    chan=         By default, if this parameter is not given, all 8 channels  \n"
"    channel=      are printed.                                                \n"
"                                                                              \n"
"    f=<format>    Select the data format.  Implemented formats are            \n"
"    fmt=          C, celsius : (default) Temperature in degrees Celsius,      \n"
"    format=       F, fahrenheit: Temperature in degrees Fahrenheit            \n"
"                  K, kelvin: Absolute temperature in Kelvin                   \n"
"                  R, raw: Raw ADC input value in the range 0..65535 (16 bit)  \n"
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
"    int           For maximum rate (0.08 approx.), set to 0 or min.           \n"
"    interval      For intervals < 0.1, the timing may be inaccurate.          \n"
"                                                                              \n"
"    n             Number of measurements to take. To take an infinite number  \n"
"    num           of measurements (default), specify 0 or \"inf\".            \n"
"    number                                                                    \n"
"                                                                              \n"
;

//-------------------------------------------------------------------------------

mbug_device thermometer = 0;

long device_serial = 0;
enum Action { Read, List, Record, Help } action = Read;
enum Format { Celsius, Fahrenheit, Kelvin, Raw } format = Celsius;

// Channel list
enum { maxchan = 100 };
int channels[maxchan+1] = {-1};

// Measurement results
double tem[8] = {0};
unsigned short raw[8] = {0};

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
	if (strncmp_upper(str,"MBUG-2810",9) == 0)
		device_serial = mbug_serial_from_id(str);
	else device_serial = str_to_uint(str);
	if (device_serial < 0) errorf( "#### Invalid device id: %s", str );
}

void parse_channel_list ( char* str )
{
	int i = 0;
	const char* s = str;
	if (str == 0) errorf( "#### Invalid channel list: %s", s );
	s += strspn( str, "01234567," );
	if (*s != 0)
		errorf( "#### Invalid entry in channel list: %s", s );
	while (i<maxchan) {
		channels[i] = str_to_uint(str);
		if (channels[i]<0 || channels[i]>7)
			errorf( "#### Invalid entry in channel list: %s", str );
		channels[++i] = -1;
		str += strcspn( str, "," );
		if (*str==0 || *++str==0) break;
	}
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
	if (!strcmp_upper(str, "MIN")) rec_interval = 0;
	else rec_interval = str_to_float(str);
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
		else if (str_in( cmd, "c", "ch", "chan", "channel", 0 ))
			parse_channel_list( arg_tok(0,"") );
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
			errorf( "#### Unknown command: %s", cmd );
	}
}


void cleanup( void )
{
	if (thermometer)
		mbug_2818_close( thermometer );
	if (rec_file) {
		fflush( rec_file );
		fclose( rec_file ); rec_file=0;
	}
}

int read_temperatures(void)
{
	int i = 0, err = 0;
	if (format==Raw)
		err = mbug_2818_read_raw( thermometer, raw, 8 );
	else
		err = mbug_2818_read_all( thermometer, tem, 8);

	for (i=0; i<8; i++)
		if (tem[i]==0.0)
			err = 1;

	if (format==Fahrenheit)
		for (i=0; i<8; i++)
			if (tem[i] > NOT_A_TEMPERATURE) 
				tem[i] = tem[i] * 9./5 + 32.;
	else if (format==Kelvin)
		for (i=0; i<8; i++)
			if (tem[i] > NOT_A_TEMPERATURE) 
				tem[i] += 273.15;

	return err;
}

//---------------------------------------------------------------

int main( int argc, char* argv[] )
{
	int i = 0, err = 0;

	atexit( cleanup );
	parse_options( argc, argv );

	if (action==Help)
		{
			puts(usage);
			return 0;
		}

	if (action==List)
		{
			// List all attached thermometers
			int i;
			mbug_device_list list = mbug_get_device_list(2818);
			for (i=0; list[i]!=0; i++ ) {
				puts(list[i]);
			}
			return 0;
		}

	// Open device
	thermometer = mbug_2818_open( device_serial );
	if (thermometer ==0 )
		errorf( "#### Error opening device." );

	// Channel list defined? .
	if (channels[0]<0)  // No -> Print all values
	{
		for (i=0; i<8; i++) channels[i] = i;
		channels[8] = -1;
	}

	//  Action: Read once
	if (action==Read)
	{
		int err = 0;
		
		err = mbug_2818_set_acq_mode( thermometer, ACQ_MODE_INST );
		if (err<0) errorf( "#### Error setting acquisition mode\n" );

		err = read_temperatures();
		if (err<0) errorf( "#### Read error\n" );

		for (i=0; channels[i]>=0 ;i++)
		{
			if (channels[i] > 7)
				channels[i] = 0;
			if (format==Raw)
				printf( "%d", raw[channels[i]] );
			else
				printf( "%.2f", tem[channels[i]] );
			printf( (channels[i+1]<0) ? "\n" : "," );
		}
	}

	if (action==Record)
	{
		int err = 0;
		double tim = 0.0, mtim = 0.0;
		char str[100] = {0};
		char sout[500] = {0};
		unsigned long nn = 0;

		if (rec_filename != 0)
		{
			rec_file = fopen( rec_filename, "a" );
			if (rec_file == 0)
				errorf( "#### Error opening file %s\n", rec_filename );
		}

		// Acquisition mode: For slow acquisitions, use synchronous mode.
		if (rec_interval > 0)
			err = mbug_2818_set_acq_mode( thermometer, ACQ_MODE_SYNC );
		else  //   For maximum acquisitions rate, use instantaneous mode (continuous background acquisition)
			err = mbug_2818_set_acq_mode( thermometer, ACQ_MODE_INST );
		if (err<0) fputs( "#### Error setting acquisition mode\n", stdout );

		// File header
		tim = floattime();
		sprintf( sout, "\n\n# %s\n# Start recording at %.2f\n# timestamp", mbug_id(thermometer), tim );
		for (i=0; channels[i]>=0 ;i++) {
			sprintf( str, "\tchan[%d]", channels[i] );
			strcat( sout, str );
		}
		strcat( sout, "\n" );
		if (rec_file)  fputs(sout, rec_file);
		if (!rec_silent)  fputs(sout, stdout);

		// Measurement loop
		tim = floattime();
		for( nn=0; (rec_number==0)||(nn<rec_number); nn++)
		{
			err = read_temperatures();
			mtim = floattime();
			if (err<0) {
				if (rec_file) fputs( "#### Read error\n", rec_file);
				fputs( "#### Read error\n", stdout );
			}

			sprintf( sout, "%.2f", mtim );
			for (i=0; channels[i]>=0 ;i++)
			{
				if (channels[i] > 7) channels[i] = 0;
				if (format==Raw)
					sprintf( str, "\t%d", raw[channels[i]] );
				else // other formats already converted by read_temperatures()
					sprintf( str, "\t%.2f", tem[channels[i]] );
				strcat( sout, str );
			}
			strcat( sout, "\n" );

			permit_abort(0); // Prevent abortion during file write to prevent data corruption
			if (rec_file)  { fputs( sout, rec_file ); fflush(rec_file); };
			if (!rec_silent) { fputs(sout, stdout); fflush(stdout); }
			permit_abort(1);
			
			if (rec_interval > 0) {
				tim += rec_interval;
				waittime( tim );
			}
		}
	}

	return 0;
}

