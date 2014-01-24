
//-----------------------------------------------------------------------
// Common utility functions for mbug vommand line tools 
//-----------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

//-----------------------------------------------------------------------

/** Prompt a formatted error message (printf format) and exit. */
static void errorf( const char *format, ... )
{
   va_list args;
   va_start( args, format );
   vfprintf( stderr, format, args );
   va_end( args );
   exit(1);
}


/** Compare a character agaisnt a list of characters given as null terminated string.
 *  Return 1 for match.
 */
static int char_in( char c, const char* set )
{
	while (*set != 0)
		if (c == *set++) return 1;
	return 0;
}


/** Compare the first string against a list of other strings, return 1 for match.
 *  The list of strings must be terminated by a null pointer or an empty string.
 */
static int str_in( const char* str, ... )
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


/** strcmp independent of character case. */
static int strcmp_upper( const char *s1, const char *s2 )
{
	unsigned int c1, c2;
	while ( (c1=toupper(*s1)) && (c2=toupper(*s2)) && c1==c2 )
		s1++, s2++;
	return c1-c2;
}


/** strncmp independent of character case (similar to the strcmpcase function which 
  * is not available on msvc)
  */
static int strncmp_upper( const char *s1, const char *s2, size_t n )
{
	unsigned int c1, c2;
	while (n-- && (c1=toupper(*s1)) && (c2=toupper(*s2)) && c1==c2)
		s1++, s2++;
	return c1-c2;
}


/** Convert string to unsigned integer (returned as signed long). In contrast to 
  * the atoi funtion, no trailing unconvertable characters are allowed. 
  * Return -1 in case of a conversion error.
  */
static long str_to_uint( const char* str )
{
	char *endp = 0;
	long val = -1;
	if (str==0 || *str=='\0') return -1;
	errno = 0;
	val = strtoul(str, &endp, 10 );
	if (errno || *endp!='\0') return -1;
	return val;
}


/** Convert string to positive float. In contrast to the atof funtion, no trailing 
  * unconvertable characters are allowed. 
  * TODO: Return NAN/INF/etc. in case of a conversion error.
  */
static double str_to_float( char* str )
{
	char *endp = 0;
	double val = -1;
	if (str==0 || *str=='\0') return -1;
	errno = 0;
	val = strtod(str, &endp );
	if (errno || *endp!='\0') return -1;
	if (val<0) return val;
	return val;
}


/** Tokenize command line parameters.
 *  First call: arg_tok( argv, 0 )  sets the arg vector to use for subsequent calls, returns nothing.
 *  Further calls: arg_tok( 0, ":=" )  returns tokens separated by one of separators ':' or '=' (like
 *      strtok() _OR_ separated by a whitespace (ie. the next argument if necessary).
 *  This allows the specification of command line parameters in the forms param=value or param value.
 */
static char* arg_tok( char**argv, const char* sep )
{
	static char *a = 0, **av = &a;
	if (argv) return av = argv, a = 0;
	if (a) if (a = strtok( 0, sep )) return a;
	return *av ? a = strtok( *av++, sep ) : 0;  // Remember: Standard requires argv[argc]==0
}
