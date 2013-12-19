
//=================================================================================

/*
	MBUG-2151 command line tool

		Mostly identical to mbug_2151_tool.c, but without support for carrier
		modulation.  Any changes in this file should also be applied in
		mbug_2151_tool.c.

*/

//=================================================================================


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
//#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#ifdef _WIN32
  #include <windows.h>
#endif

#include "mbug_2151.h"
#include "mbug_2151_targets.h"

//----------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"MBUG-2151 command line tool                                                   \n"
"                                                                              \n"
"Syntax:   mbug_2151  cmd, cmd2, ...                                           \n"
"                                                                              \n"
"    For convenience, most commands have multiple abbrevations.  Commands can  \n"
"    be prepended by '-' (unix option style),  '/' (windows option style),     \n"
"    or nothing.  Some commands take a parameter. The parameter has to be      \n"
"    separeted by a ':', '=', or whitespace.                                   \n"
"                                                                              \n"
"    If a transmission sequence is specified (see below), the the program      \n"
"    first stops the device gracefully (see below) and waits until the         \n"
"    currently active transmission is complete.  If the force command is       \n"
"    specified, the device is stopped immediately (possibly cancelling the     \n"
"    current transmssion.)  After the device becomes ready, the newly          \n"
"    specified sequence is uploaded and started.                               \n"
"                                                                              \n"
"Commands:                                                                     \n"
"                                                                              \n"
"    r            Reset device.                                                \n"
"    reset                                                                     \n"
"                                                                              \n"
"    l            List attached devices.                                       \n"
"    ls                                                                        \n"
"    list                                                                      \n"
"                                                                              \n"
"    d=<id>       Select device with specified serial number or id string (as  \n"
"    dev=         returned by the list command). If no device is specified,    \n"
"    device=      the first available device is used.                          \n"
"                                                                              \n"
"    s=t0,t1,...  Set transmission sequence.  Items, separated by ',' or ';'   \n"
"                 are interpreted as interval times in milliseconds.           \n"
"                 Always specify an even number of inervals. Maximum number    \n"
"                 of intervals is 254.  The maximum interval time is 600 sec   \n"
"                 for < 128 intervals, 4 sec for >=128 intervals.              \n"
"                 Highest posible time resolution is 1 micosecond, but         \n"
"                 decreases with the maximum of the sequence time interval.    \n"
"                                                                              \n"
"    i=<iterations>   Number of iterations (number of times the complete       \n"
"    it=              sequence is repeated).  0: infinite iterations           \n"
"    iter                                                                      \n"
"                                                                              \n"
"    w            If a sequence is specified, wait until the sequence has      \n"
"    wait         been transmitted completely.  If no sequence is specified,   \n"
"                 wait until the currently active transmission of the device   \n"
"                 is completed.  NOTE: If iterations is set to infinity,       \n"
"                 this command is ignored.                                     \n"
"                                                                              \n"
"    q            Quit a currently active transmission gracefully, i.e. after  \n"
"    quit         the current iteration and wait until it has finished.  Use   \n"
"                  together with force to stop immediately.                    \n"
"                                                                              \n"
"    f            Force quit! If quit has been specified, the device is forced \n"
"    force        to quit the currently running transmission ungracefully.     \n"
"                 If a sequence is specified, the device is stopped            \n"
"                 ungracefully before the new sequence is started.             \n"
"                                                                              \n"
"    b            Query if the selected device is currently busy transmitting  \n"
"    busy         or not.  The busy state is returned as 1/0 on stddout.       \n";

//-----------------------------------------------------------------------------------

long device_serial = 0;
enum Action { Help, Send, List, Wait, Quit, Busy, Reset, None } action = Send;

double mod_freq = 38e3;		// Modulation frequency
double timebase = 1e-3;		// timebase for sequence
long iterations = 3;			// Number of iteration
unsigned char seq_bytes[255];// Sequence bytes
int seq_length = 0;			// Length of sequence in bytes
int tx_mode = TX_MODE_TIMED_16;	// Transmission mode
int force = 0;				// Force stop ungracefully

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
	while(1) {
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

double str_to_float( char* str )
{
	char *endp = 0;
	double val = -1;
	if (str==0 || *str=='\0') return -1;
	errno = 0;
	val = strtod(str, &endp );
	if (errno || *endp!='\0') return -1;
	return val;
}

char *tokenize(tokenize_state_t *state, char *argv[], const char *sep)
{
	char **s = *(char***) state;
	char *arg = NULL;
	char *ret;
	if(state == NULL) {
		return NULL;
	}
	if(argv != NULL) {
		*state = argv;
		s = argv;
		arg = *s;
	}

	if(*s == NULL) {
		return NULL;
	}

	ret = strtok(arg, sep);
	while(ret == NULL) {
		s++;
		if((*s) == NULL) {
			*state = s;
			return NULL;
		}
		ret = strtok(*s, sep);
	}

	*(char ***)state = s;
	return ret;
}

void parse_device_id( char* str )
{
	str_toupper( str );
	if (strncmp(str,"MBUG-2151",9) == 0)
		device_serial = mbug_serial_from_id(str);
	else device_serial = str_to_int(str);
	if (device_serial < 0) errorf( "#### Invalid device id: %s", str );
}

void parse_iterations( char* str )
{
	str_toupper( str );
	if (strcmp(str,"INF") == 0)
		iterations = 0;
	else if ((iterations = str_to_int( str )) < 0)
		errorf( "#### Invalid iterations: %s", str );
}

//---------------------------------------------------------------

void parse_sequence( char* sseq )
{
	size_t  offs = 0;
	size_t  slen = strlen(sseq);
	double fseq[255];	// Sequence in double format
	double mini, maxi;	// Minium/maximum sequence items
	int ni = 0;			// Number of sequence items
	double bi = 1e-6;	// Base interval
	int ii;

	if (sseq==NULL) return;
	if (slen==0) return;

	// Convert sequence string to floating point
	while (offs<slen) {
		fseq[ni] = atof(sseq+offs)*1e-3;	// sequence is given in units of milliseconds
		if (fseq[ni] == 0.0)
			errorf("#### Invalid sequence item at position %d: %s\n", ni, sseq+offs);
		if (++ni >= 255)
			errorf("#### Sequence too long (max 255 items)");
		offs += 1 + strcspn( sseq+offs, ",;" );
	}

	// Get minimum/maximum sequence item
	mini = maxi = fseq[0];
	for (ii=1; ii<ni; ii++)	{
		if (fseq[ii] < mini) mini=fseq[ii];
		if (fseq[ii] > mini) maxi=fseq[ii];
	}
	if (maxi > 600.)
		errorf("#### Maximum interval is 600 seconds");

	// <= 127 items? -> Use 16 bit mode
	if (ni<128) {
		// Determine reqired base interval
		bi = 1e-6;
		while (maxi>65535*bi)
			bi *= 10;

		// Assemble sequence bytes
		for (ii=0; ii<ni; ii++)  {
			int item = (int)(0.5 + fseq[ii]/bi);
			if (item<1) item = 1;
			seq_bytes[2*ii] = item & 0xff;
			seq_bytes[2*ii+1] = (item>>8) & 0xff;
		}
		tx_mode = TX_MODE_TIMED_16;
		seq_length = ni*2;
		timebase = bi;
	}
	else {	// ni>127 -> Use 8 bit mode
		if (maxi > 4.)
			errorf("#### Intervals > 4 seconds are not possible with more than 127 sequence items.");

		// Determine reqired base interval
		bi = 1e-6;
		while (maxi>255*bi)
			bi *= 2;

		// Assemble sequence bytes
		for (ii=0; ii<ni; ii++) {
			int item = (int)(0.5 + fseq[ii]/bi);
			if (item<1) item = 1;
			seq_bytes[ii] = item & 0xff;
		}
		tx_mode = TX_MODE_TIMED_8;
		seq_length = ni;
		timebase = bi;
	}
}

//---------------------------------------------------------------

/* Expected command format:
   id{,id}* (on|1|off|0)
*/
void parse_target_ab440s(tokenize_state_t *s) {
	char *id;
	int state, i;
	mbug_device transmitter;
	double it_time;

	char *ids = tokenize(s, NULL, ""); // Store the next string in ids for later use
	char *statestr = tokenize(s, NULL, "");

	if(ids == NULL || statestr == NULL) {
		errorf("Usage: AB440S <id,id,...> (on|1|off|0)\n");
	}

	if(str_in(statestr, "1", "on", 0)) {
		state = 1;
	} else if(str_in(statestr, "0", "off", 0)) {
		state = 0;
	} else {
		errorf("Usage: AB440S <id1,id2,...> (on|1|off|0)\n");
		return;
	}

	transmitter = mbug_2151_open( device_serial );
	while(transmitter == NULL) {
		printf("Meh!\n");
		transmitter = mbug_2151_open(device_serial);
	}
	while(mbug_2151_get_busy(transmitter)) {
		int length = mbug_2151_get_seq_length(transmitter);

		if(length == -1) {
			mbug_2151_close(transmitter);
			transmitter = mbug_2151_open( device_serial );
			printf("error, retrying\n");
		}

		if(mbug_2151_get_iterations(transmitter) == 0) {
			//printf("killing\n");
			mbug_2151_stop_gracefully(transmitter);
		}
		it_time = (double) length * 5
			* mbug_2151_get_clock_div(transmitter)
			/ mbug_2151_get_base_clock(transmitter);
		#ifdef _WIN32
			Sleep( (int)(1. + it_time*1e3));
		#else
			struct timespec ts;
			ts.tv_sec = (int) it_time;
			ts.tv_nsec = (it_time - ts.tv_sec) * 1e9;
			nanosleep(&ts, NULL);
		#endif
	}

	mbug_2151_set_bitrate(transmitter, 3125);
	mbug_2151_set_iterations(transmitter, 3);

	seq_length = 16;

	for(i = 0; i < 12; i++) {
		seq_bytes[i] = 0x71;
	}
	if(state == 1) {
		seq_bytes[10] = 0x11;
	} else {
		seq_bytes[11] = 0x11;
	}
	seq_bytes[12] = 1;
	seq_bytes[13] = seq_bytes[14] = seq_bytes[15] = 0;

	for(id = strtok(ids, ",");
		id != NULL;
		id = strtok(NULL, ",")) {
		for(i = 0; i < 10; i++) {
			seq_bytes[i] = 0x71;
		}
		for(; *id; id++) {
			if(*id >= '1' && *id <= '5') {
				seq_bytes[*id - '1'] = 0x11;
			} else if(toupper(*id) >= 'A' && toupper(*id) <= 'E') {
				seq_bytes[5 + toupper(*id) - 'A'] = 0x11;
			}
		}
		//printf("Sending ");
		//for(int i = 0; i < 16; i++) {
		//	printf("%02x ", seq_bytes[i]);
		//}
		//printf("\n");
		mbug_2151_set_sequence(transmitter, seq_bytes, 16, TX_MODE_BITSTREAM);
		mbug_2151_start(transmitter);
	}
	mbug_2151_close(transmitter);
}

void parse_target_he302eu(tokenize_state_t *s) {

}

void parse_target_dmv7008(tokenize_state_t *s) {

}

void parse_target_ikt201(tokenize_state_t *s) {

}

target_parse_func_type *get_target_parse_function(char *target)
{
	int i;
	for(i = 0; i < NUM_MBUG_2151_TARGETS; i++) {
		if(!strcmp(str_toupper(target), mbug_2151_targets[i].name)) {
			return mbug_2151_targets[i].parse_function;
		}
	}
	return NULL;
}

int is_target( char *token )
{
	return get_target_parse_function(token) != NULL;
}

//---------------------------------------------------------------

void parse_options( int argc, char* argv[] )
{
	tokenize_state_t st;
	char *ap;
	for (ap = tokenize(&st, argv+1, "-/"); ap; ap = tokenize(&st, NULL, "-/"))
	{
		if (str_in( ap, "l", "list", 0 ))
			action = List;
		else if (str_in( ap, "h", "help", 0))
			action = Help;
		else if (str_in( ap, "d", "dev", "device", 0 ))
			parse_device_id(tokenize(&st, NULL, ""));
		else if (str_in( ap, "r", "reset", 0 ))
			action = Reset;
		else if (str_in( ap, "w", "wait", 0 ))
			action = Wait;
		else if (str_in( ap, "q", "quit", "stop" , 0 ))
			action = Quit;
		else if (str_in( ap, "b", "busy", 0 ))
			action = Busy;
		else if (str_in( ap, "i", "it", "iter", 0 ))
			parse_iterations(tokenize(&st, NULL, ""));
		else if (str_in( ap, "s", "seq", 0 ))
			parse_sequence(tokenize(&st, NULL, ""));
		else if (is_target(ap)) {
			action = None;
			get_target_parse_function(ap)(&st);
			return;
		}
		else
			errorf( "#### Unknown command: %s", ap);
	}
}

//---------------------------------------------------------------

int main( int argc, char* argv[] )
{
	mbug_device transmitter = 0;

	parse_options( argc, argv );

	if (action==List)
		{
			// List all attached thermometers
			int i;
			mbug_device_list list = mbug_get_device_list(2151);
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

	if (action == None) {
		return 0;
	}

	transmitter = mbug_2151_open( device_serial );
	if (transmitter == 0)
		errorf("#### Error opening device.");

	if (action==Reset)
	{
		mbug_2151_reset( transmitter );
	}
	else if (action==Quit)
	{
		if (force)
			mbug_2151_stop_immediately( transmitter );
		else {
			mbug_2151_stop_gracefully( transmitter );
			while (mbug_2151_get_busy( transmitter ))
				/*wait*/ ;
		}
	}
	else if (action==Busy)
	{
		int busy = mbug_2151_get_busy( transmitter );
		if (busy) puts("1");
		else puts("0");
	}
	if (action==Send || action==Wait)
	{
		// Sequence specified?
		if (seq_length>0)
		{
			// Stop currently active transmission
			if (force)
				mbug_2151_stop_immediately( transmitter );
			else {
				mbug_2151_stop_gracefully( transmitter );
				while (mbug_2151_get_busy( transmitter ))
					/*wait*/ ;
			}

			mbug_2151_reset( transmitter );
			mbug_2151_set_interval( transmitter, timebase );
			mbug_2151_set_iterations( transmitter, iterations );
			mbug_2151_set_sequence( transmitter, seq_bytes, seq_length, tx_mode );
			mbug_2151_start( transmitter );
		}

		if (action==Wait)
		{
			int iter = mbug_2151_get_iterations( transmitter );
			if (iter)  // Only wait if not set to infinite iterations
				while (mbug_2151_get_busy( transmitter ))
					/*wait*/ ;
		}
	}

	mbug_2151_close(transmitter);
	return 0;
}

