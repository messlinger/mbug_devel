
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mbug.h"

//-------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"LSMBUG command line tool: List available mbug devices.                        \n"
"                                                                              \n"
"Usage:   lsmbug  [cmd]                                                        \n"
"                                                                              \n"
"    For convenience, commands have multiple abbrevations.  Commands can       \n"
"    be prepended by on or multiple '-' (unix option style),  '/' (windows     \n"
"    option style), or nothing.  Some commands take parameters, which have     \n"
"    to be separeted from the command by a ':', '=' or a whitespace.           \n"
"                                                                              \n"
"Commands:                                                                     \n"
"                                                                              \n"
"    t=<type>      Select device type to list.                                 \n"
"    type=         Default: all types.                                         \n"
"                                                                              \n"
"    h             Display this usage info.                                    \n"
"    help                                                                      \n";

//--------------------------------------------------------------------------------

int device_type = 0;
enum Action { List, Help } action = List;

//---------------------------------------------------------------

/** Prompt a formatted error message (printf format) and exit. */
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
 *  Returns -1 in cae of a conversion error. 
 */
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

/** Extract device type from string. */
void parse_type( char* str )
{
	if (strncmp_upper(str,"MBUG-",5) == 0)
		device_type = str_to_uint(str+5);
	else device_type = str_to_uint(str);
	if (device_type < 0) errorf( "#### Invalid device type: %s", str );
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
		else if (str_in( cmd, "h", "help", 0 ))
			action = Help;
		else if (str_in( cmd, "t", "type", 0 ))
			parse_type( arg_tok(0,"") );
		else
			errorf( "#### Unknown command: %s", cmd );
	}
}

//---------------------------------------------------------------

int main( int argc, char* argv[] )
{
	parse_options( argc, argv );
	
	if (action==List)
		{
			// List all attached devices
			int i;
			mbug_device_list list = mbug_get_device_list(device_type);
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

	return 0;
}

