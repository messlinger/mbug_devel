
#include "mbug_2820.h"
#include "mbug_utils.h"

//-------------------------------------------------------------------------------

const char* usage =
"                                                                              \n"
"MBUG-2820 command line tool                                                   \n"
"                                                                              \n"
"Usage:   mbug_2820  cmd, cmd2, ...                                            \n"
"                                                                              \n"
"    For convenience, commands have multiple abbrevations.  Commands can       \n"
"    be prepended by on or multiple '-' (unix option style),  '/' (windows     \n"
"    option style), or nothing.  Some commands take parameters, which have     \n"
"    to be separeted from the command by a ':', '=' or a whitespace.           \n"
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


/** Extract device serial. */
void parse_device_id( char* str )
{
	if (strncmp_upper(str,"MBUG-2810",9) == 0)
		device_serial = mbug_serial_from_id(str);
	else device_serial = str_to_uint(str);
	if (device_serial < 0) errorf( "#### Invalid device id: %s", str );
}

/** Parse the argv for command line parameters. */
void parse_options( int argc, char* argv[] )
{
	char *cmd;
	arg_tok( argv+1, 0 );
	while (cmd = arg_tok(0,":="))
	{
		if (str_in( cmd, "l", "ls", "list", 0 ))
			action = List;
		else if (str_in( cmd, "r", "rd", "read", 0 ))
			action = Read;
		else if (str_in( cmd, "h", "help", 0 ))
			action = Help;
		else if (str_in( cmd, "d", "dev", "device", 0 ))
			parse_device_id( arg_tok(0,"") );
		else
			errorf( "#### Unknown command: %s", cmd );
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

