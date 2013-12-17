
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mbug_2810.h"

//-------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"MBUG-2810 command line tool                                                   \n"
"                                                                              \n"
"Usage:   mbug_2810  cmd, cmd2, ...                                            \n"
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
"    h             Display this usage info.                                    \n"
"    help                                                                      \n";

//--------------------------------------------------------------------------------

int device_serial = 0;
enum Action { Read, List, Help } action = Read;
enum Format { Celsius, Fahrenheit, Kelvin, Raw } format = Celsius;

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
	if (strncmp(str,"MBUG-2810",9) == 0)
		device_serial = mbug_serial_from_id(str);
	else device_serial = str_to_int(str);
	if (device_serial < 0) errorf( "#### Invalid device id: %s", str );
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
		else if (str_in( ap, "f", "fmt", "format", 0 ))
			parse_format( strtok( 0, "" ) );
		else
			errorf( "#### Unknown command: %s", argv[i]);
	}
}

//---------------------------------------------------------------

int main( int argc, char* argv[] )
{
	mbug_device thermometer = 0;

	parse_options( argc, argv );

	if (action==List)
		{
			// List all attached thermometers
			int i;
			mbug_device_list list = mbug_get_device_list(2810);
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
	thermometer = mbug_2810_open( device_serial );
	if (thermometer ==0 )
		errorf("#### Error opening device.");

	if (action==Read)
	{
		double tem;
		int raw;
		if (format==Raw)
			raw = mbug_2810_read_raw( thermometer );
		else
			tem = mbug_2810_read( thermometer );

		if (format==Fahrenheit)
				tem = tem * 9./5 + 32.;
		else if (format==Kelvin && tem >= -273.15)
				tem += 273.15;

		// print
		if (format==Raw)
			printf( "%d", raw );
		else
			printf( "%.2f", tem );
	}

	mbug_2810_close(thermometer);
	return 0;
}

