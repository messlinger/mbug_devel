
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
"    h             Display this usage info.                                    \n"
"    help                                                                      \n";

//-------------------------------------------------------------------------------
long device_serial = 0;
enum { maxchan = 100 };
int channels[maxchan+1] = {-1};
enum Format { Celsius, Fahrenheit, Kelvin, Raw } format = Celsius;
enum Action { Read, List, Help } action = Read;


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

/** Parse the argv for command line parameters. */
void parse_options( int argc, char* argv[] )
{
	char *cmd;
	arg_tok( argv+1, 0 );
	while (cmd = arg_tok(0,":="))
	{
		cmd += strspn( cmd, "-/" );
		if (str_in( cmd, "l", "ls", "list", 0 ))
			action = List;
		else if (str_in( cmd, "r", "rd", "read", 0 ))
			action = Read;
		else if (str_in( cmd, "h", "help", 0 ))
			action = Help;
		else if (str_in( cmd, "d", "dev", "device", 0 ))
			parse_device_id( arg_tok(0,"") );
		else if (str_in( cmd, "c", "ch", "chan", "channel", 0 ))
			parse_channel_list( arg_tok(0,"") );
		else if (str_in( cmd, "f", "fmt", "format", 0 ))
			parse_format( arg_tok(0,"") );
		else
			errorf( "#### Unknown command: %s", cmd );
	}
}

//---------------------------------------------------------------

int main( int argc, char* argv[] )
{
	int i = 0;
	mbug_device thermometer = 0;
	double tem[8] = {0};
	unsigned short raw[8] = {0};

	parse_options( argc, argv );

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

	if (action==Help)
		{
			puts(usage);
			return 0;
		}

	// Open device
	thermometer = mbug_2818_open(device_serial);
	if (thermometer ==0 )
		errorf( "#### Error opening device." );

	if (action==Read)
	{
		if (format==Raw)
			mbug_2818_read_raw( thermometer, raw, 8 );
		else
			mbug_2818_read_all( thermometer, tem, 8);

		if (format==Fahrenheit)
			for (i=0; i<8; i++)
				tem[i] = tem[i] * 9./5 + 32.;
		else if (format==Kelvin)
			for (i=0; i<8; i++)
				if (tem[i] >= -273.15)
					tem[i] += 273.15;

		if (channels[0]<0)
		{
			// print all values
			for (i=0; i<8; i++) channels[i] = i;
			channels[8] = -1;
		}
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

	mbug_2818_close(thermometer);
	return 0;
}

