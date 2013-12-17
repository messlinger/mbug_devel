
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#include "mbug_2818.h"

//-------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"MBUG-2818 command line tool                                                   \n"
"                                                                              \n"
"Usage:   mbug_2818  cmd, cmd2, ...                                            \n"
"                                                                              \n"
"    For convenience, most commands have multiple abbrevations.  Commands can  \n"
"    be prepended by on or multiple '-' (unix option style),  '/' (windows     \n"
"    option style), or nothing.  Some commands take a parameter. The parameter \n"
"    has to be  separeted by a ':' or '=' (NO whitespaces!).                   \n"
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

void errorf( const char *format, ... ) 
{
   va_list args;
   va_start( args, format );
   vfprintf( stderr, format, args );
   va_end( args );
   exit(1);
}

int char_in( char c, const char* set ) 
{
	while (*set != 0) 
		if (c == *set++) return 1;
	return 0;
}

int str_in( const char* str, ... )
{
	va_list args;
	int match = 0;
	va_start(args, str);
	while(1) 
	{
		const char *next = va_arg(args, const char*);
		if (next==0 || *next==0) break;
		if (match = !strcmp(str,next)) break;
	} 
	va_end(args);
	return match;
}

char *str_toupper( char *str )
{
	char *p = str;
	while (*p) {
		*p = toupper(*p);
		p++;
	}
	return str;
}

long str_to_int( char* str )
{
	char *endp = 0;
	long val = -1;
	if (str==0 || *str=='\0') return -1;
	errno = 0;
	val = strtoul(str, &endp, 10 );
	if (errno || *endp!='\0') return -1;
	return val;
}

void parse_device_id( char* str )
{
	str_toupper( str );
	if (strncmp(str,"MBUG-2818",9) == 0)
		device_serial = mbug_serial_from_id(str);
	else device_serial = str_to_int(str);
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
		channels[i] = str_to_int(str);
		if (channels[i]<0 || channels[i]>7)
			errorf( "#### Invalid entry in channel list: %s", str );
		channels[++i] = -1;
		str += strcspn( str, "," );
		if (*str==0 || *++str==0) break;
	}
}

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

void parse_options( int argc, char* argv[] )
{ 
	int i;
	for (i=1; i<argc; i++)
	{
		char* ap = argv[i];
		while ( *ap=='-' || *ap=='/' )  ap++;
		ap = strtok( ap, ":= ");

		if (str_in( ap, "l", "list", 0 ))
			action = List;
		else if (str_in( ap, "r", "rd", "read", 0 ))
			action = Read;
		else if (str_in( ap, "h", "help", 0 ))
			action = Help;
		else if (str_in( ap, "d", "dev", "device", 0 ))
			parse_device_id( strtok( 0, "" ) );
		else if (str_in( ap, "c", "ch", "chan", "channel", 0 ))
			parse_channel_list( strtok( 0, "" ) );
		else if (str_in( ap, "f", "fmt", "format", 0 ))
			parse_format( strtok( 0, "" ) );
		else 
			errorf( "#### Unknown command: %s", argv[i]);
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

