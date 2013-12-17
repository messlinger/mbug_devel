
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mbug_2820.h"

//-------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"MBUG-2820 command line tool                                                   \n"
"                                                                              \n"
"Usage:   mbug_2820  cmd, cmd2, ...                                            \n"
"                                                                              \n"
"    For convenience, most commands have multiple abbrevations.  Commands can  \n"
"    be prepended by on or multiple '-' (unix option style),  '/' (windows     \n"
"    option style), or nothing.  Some commands take a parameter. The parameter \n"
"    has to be  separeted by a ':' or '=' (NO whitespaces!).                   \n"
"                                                                              \n"
"Commands:                                                                     \n"
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
"    d=<id>        Select device with specified serial number or id string (as \n"
"    dev=          returned by the list command). If no device is specified,   \n"
"    device=       the first available device is used.                         \n"
"                                                                              \n"
"    h             Display this usage info.                                    \n"
"    help                                                                      \n";

//--------------------------------------------------------------------------------

int device_serial = 0;
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
	if (strncmp(str,"MBUG-2820",9) == 0)
		device_serial = mbug_serial_from_id(str);
	else device_serial = str_to_int(str);
	if (device_serial < 0) errorf( "#### Invalid device id: %s", str );
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
	thermometer = mbug_2820_open( device_serial );
	if (thermometer ==0 )
		errorf("#### Error opening device.");

	if (action==Read)
		{
			double tem, hum;
			mbug_2820_read( thermometer, &tem, &hum );
			printf( "%.2f,%.2f\n", tem, hum );
		}

	mbug_2820_close(thermometer);
	return 0;
}

