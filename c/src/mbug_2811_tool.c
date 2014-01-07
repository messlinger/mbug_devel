
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mbug_2811.h"

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

/** Print a formatted error message (printf format) and exit. */
void errorf( const char *format, ... )
{
   va_list args;
   va_start( args, format );
   vfprintf( stderr, format, args );
   va_end( args );
   exit(1);
}

/** Compare the first string against a list of other strings, return 1 for match.
 *  The list of strings must be terminated by a null pointer or an empty string. 
 */
int str_in( const char* str, ... )
{
	va_list args;
	int match = 0;
	va_start(args, str);
	while(1) {
		const char *next = va_arg(args, const char*);
		if (next==0 || *next==0) break;
		if (match = !strcmp(str,next)) break;
	}
	va_end(args);
	return match;
}

/** strncmp independent of character case. */
int strncmp_upper( const char *str1, const char *str2, size_t n )
{
	int left, right;
	while (n-- && (left=toupper(*str1)) && (right=toupper(*str2))) {
		if (left==right) str1++, str2++;
		else return left-right;
	}
	return 0;
}

/** Convert string to unsigned integer. No trailing unconvertable characters are allowed.
 *  Returns -1 in cae of a conversion error. */
long str_to_uint( char* str )
{
	char *endp = 0;
	long val = -1;
	if (str==0 || *str=='\0') return -1;
	errno = 0;
	val = strtoul(str, &endp, 10 );
	if (errno || *endp!='\0') return -1;
	return val;
}

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

/** Tokenize command line parameters.
 *  First call: arg_tok( argv, 0 )  sets the arg vector to use for subsequent calls, returns nothing.
 *  Further calls: arg_tok( 0, ":=" )  returns tokens separated by one of separators ':' or '=' (like
 *      strtok() _OR_ separated by a whitespace (ie. the next argument if necessary).
 *  This allows the specification of command line parameters in the forms param=value or param value. 
 */
char* arg_tok( char**argv, const char* sep )
{
	static char *a = 0, **av = &a;
	if (argv) return av = argv, a = 0;
	if (a) if (a = strtok( 0, sep )) return a;
	return *av ? a = strtok( *av++, sep ) : 0;  // Remember: Standard requires argv[argc]==0 
}

/** Parse the argv for command line parameters. */
void parse_options( int argc, char* argv[] )
{
	char *cmd;
	arg_tok( argv+1, 0 );
	while (cmd = arg_tok(0,":="))
	{
		cmd += strspn( cmd, "-/" );
		if (str_in( cmd, "l", "list", 0 ))
			action = List;
		else if (str_in( cmd, "r", "rd", "read", 0 ))
			action = Read;
		else if (str_in( cmd, "h", "help", 0 ))
			action = Help;
		else if (str_in( cmd, "d", "dev", "device", 0 ))
			parse_device_id( arg_tok(0,"") );
		else if (str_in( cmd, "f", "fmt", "format", 0 ))
			parse_format( arg_tok(0,"") );
		else
			errorf( "#### Unknown command: %s", cmd);
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
			mbug_device_list list = mbug_get_device_list(2811);
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
	thermometer = mbug_2811_open( device_serial );
	if (thermometer ==0 )
		errorf("#### Error opening device.");

	if (action==Read)
	{
		double tem;
		int raw;
		if (format==Raw)
			raw = mbug_2811_read_raw( thermometer );
		else
			tem = mbug_2811_read( thermometer );

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

	mbug_2811_close(thermometer);
	return 0;
}

