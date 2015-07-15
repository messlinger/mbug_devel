
//-----------------------------------------------------------------------
// Common utility functions for mbug command line tools
//-----------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/timeb.h>
#ifdef _WIN32
	#include <windows.h>
#elif __linux__
	#include <unistd.h>
#else
	#error "OS not supported"
#endif


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
	while ( (c1=toupper(*s1) , c2=toupper(*s2)) && c1==c2 )
		s1++, s2++;
	return c1-c2;
}


/** strncmp independent of character case (similar to the strcmpcase function which
  * is not available on msvc)
  */
static int strncmp_upper( const char *s1, const char *s2, size_t n )
{
	unsigned int c1, c2;
	while (n-- && (c1=toupper(*s1) , c2=toupper(*s2)) && c1==c2)
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
static double str_to_float( const char* str )
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


/** Get current time with millisecond resolution.
    NOTE: On Windows, the timestamp is only updated every 10 or 15 milliseconds.
 */
double floattime( void )
{
	#ifdef _WIN32
		struct timeb t = {0};
		ftime(&t);
		return t.time + 1e-3 * t.millitm;
	#elif __linux__
		struct timeval tv = {0};
		gettimeofday( &tv, 0 );
		return tv.tv_sec + 1e-6*tv.tv_usec;
	#endif
}


/** Wait until specified timestamp with sub-second resolution. */
void waittime( double timestamp )
{
	double delta;
	while ( (delta = timestamp - floattime()) > 0)
		if (delta >= 1e-3)
			#ifdef _WIN32
				Sleep( delta > 0.1 ? 80 : (DWORD)(0.8*1e3*delta));	// Will return at next time update interrupt
			#elif __linux__
				usleep( delta > 0.1 ? 80000 : (useconds_t)(0.8*1e6*delta) );
			#endif
}


/* -----------------------------------------------------------------------------------------------------
 * Want to be able to abort the program, even during blocking calls (sleep/read), but NOT during file 
 * output to prevent data corruption. Install a signal handler, that only aborts if permitted. If not
 * permitted to abort, remember the signal and exit immediately when permitted again.
 */

volatile struct { 
	unsigned permit:1, abort : 1;
	} permit_abort_flag = { 1, 0 };

#ifdef _WIN32
	BOOL permit_abort_handler( DWORD fdwCtrlType ) 
	{
		if (permit_abort_flag.permit) return FALSE; // Will delegate to normal handler and abort
		permit_abort_flag.abort = 1;
		return TRUE; // Will not abort
	}
#elif __linux__
#endif

void permit_abort(int permit)
{
	static int handler_registered = 0;
	if (!handler_registered)
		#ifdef _WIN32  
			SetConsoleCtrlHandler( (PHANDLER_ROUTINE) permit_abort_handler, TRUE );
		#elif __linux__
		#endif
	permit_abort_flag.permit = permit ? 1 : 0;
	if (permit && permit_abort_flag.abort) exit(0);
}
