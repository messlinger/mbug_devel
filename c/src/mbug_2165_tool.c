
#include "mbug_2165.h"
#include "mbug_utils.h"

//----------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"MBUG-2165 command line tool                                                   \n"
"                                                                              \n"
"Syntax:   mbug_2165  cmd, cmd2, ...                                           \n"
"                                                                              \n"
"    For convenience, most commands have multiple abbrevations.  Commands can  \n"
"    be prepended by '-' (unix option style),  '/' (windows option style),     \n"
"    or nothing.  Some commands take a parameter. The parameter has to be      \n"
"    separeted by a ':' or '=' (NO whitespaces!).                              \n"
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
"    it=              sequence is repeated).  0, inf: infinite iterations      \n"
"    iter=                                                                     \n"
"                                                                              \n"
"    c=<freq>     Carrier frequency in kHz.  0: No modulation.                 \n"
"    carrier=     Default is 38 kHz.                                           \n"
"                                                                              \n"
"    w            If a sequence is specified, wait until the sequence has      \n"
"    wait         been transmitted completely.  If no sequence is specified,   \n"
"                 wait until the currently active transmission of the device   \n"
"                 is completed.  NOTE: If iterations is set to infinity,       \n"
"                 this command is ignored.                                     \n"
"                                                                              \n"
"    q            Quit a currently active transmission gracefully, i.e. after  \n"
"    quit         the current iteration and wait until it has finished.  Use   \n"
"                 together with force to stop immediately.                     \n"
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
enum Action { Help, Send, List, Wait, Quit, Busy, Reset } action = Send;

double mod_freq = 38e3;		// Modulation frequency
double timebase = 1e-3;		// timebase for sequence
long iterations = 3;			// Number of iteration
unsigned char seq_bytes[255];// Sequence bytes
int seq_length = 0;			// Length of sequence in bytes
int tx_mode = TX_MODE_TIMED_16;	// Transmission mode
int force = 0;				// Force stop ungracefully

//---------------------------------------------------------------

void parse_device_id( char* str )
{
	if (strncmp_upper(str,"MBUG-2165",9) == 0)
		device_serial = mbug_serial_from_id(str);
	else device_serial = str_to_uint(str);
	if (device_serial < 0) errorf( "#### Invalid device id: %s", str );
}

void parse_iterations( char* str )
{
	if (strcmp_upper(str,"INF") == 0)
		iterations = 0;
	else if ((iterations = str_to_uint( str )) < 0)
		errorf( "#### Invalid iterations: %s", str );
}

void parse_carrier( char* str )
{
	if ((mod_freq = 1e3*str_to_float( str )) < 0)
		errorf( "#### Invalid carrier frequency: %s", str );
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

void parse_options( int argc, char* argv[] )
{
	int i;
	for (i=1; i<argc; i++)
	{
		char* ap = argv[i];
		while ( *ap=='-' || *ap=='/' ) ap++;
		ap = strtok( ap, ":= ");

		if (str_in( ap, "l", "ls", "list", 0 ))
			action = List;
		else if (str_in( ap, "h", "help", 0))
			action = Help;
		else if (str_in( ap, "d", "dev", "device", 0 ))
			parse_device_id( strtok( 0, "" ) );
		else if (str_in( ap, "r", "reset", 0 ))
			action = Reset;
		else if (str_in( ap, "w", "wait", 0 ))
			action = Wait;
		else if (str_in( ap, "q", "quit", "stop" , 0 ))
			action = Quit;
		else if (str_in( ap, "b", "busy", 0 ))
			action = Busy;
		else if (str_in( ap, "i", "it", "iter", 0 ))
			parse_iterations( strtok( 0, "" ) );
		else if (str_in( ap, "s", "seq", 0 ))
			parse_sequence( strtok( 0, "" ) );
		else if (str_in( ap, "c", "carrier", 0 ))
			parse_carrier( strtok( 0, "" ) );
		else
			errorf( "#### Unknown command: %s", argv[i]);
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
			mbug_device_list list = mbug_get_device_list(2165);
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
	transmitter = mbug_2165_open( device_serial );
	if (transmitter == 0)
		errorf("#### Error opening device.");

	if (action==Reset)
	{
		mbug_2165_reset( transmitter );
	}
	else if (action==Quit)
	{
		if (force)
			mbug_2165_stop_immediately( transmitter );
		else {
			mbug_2165_stop_gracefully( transmitter );
			while (mbug_2165_get_busy( transmitter ))
				/*wait*/ ;
		}
	}
	else if (action==Busy)
	{
		int busy = mbug_2165_get_busy( transmitter );
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
				mbug_2165_stop_immediately( transmitter );
			else {
				mbug_2165_stop_gracefully( transmitter );
				while (mbug_2165_get_busy( transmitter ))
					/*wait*/ ;
			}

			mbug_2165_reset( transmitter );
			mbug_2165_set_mod_freq( transmitter, mod_freq );
			mbug_2165_set_timebase( transmitter, timebase );
			mbug_2165_set_iterations( transmitter, iterations );
			mbug_2165_set_sequence( transmitter, seq_bytes, seq_length, tx_mode );
			mbug_2165_start( transmitter );
		}

		if (action==Wait)
		{
			int iter = mbug_2165_get_iterations( transmitter );
			if (iter)  // Only wait if not set to infinite iterations
				while (mbug_2165_get_busy( transmitter ))
					/*wait*/ ;
		}
	}

	mbug_2165_close(transmitter);
	return 0;
}

