

#include "mbug.h"
#include "mbug_utils.h"

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

/** Extract device type from string. */
void parse_type( char* str )
{
	if (strncmp_upper(str,"MBUG-",5) == 0)
		device_type = str_to_uint(str+5);
	else device_type = str_to_uint(str);
	if (device_type < 0) errorf( "#### Invalid device type: %s", str );
}


/** Parse the argv for command line parameters. */
void parse_options( int argc, char* argv[] )
{
	char *cmd;
	arg_tok( argv+1, 0 );
	while (cmd = arg_tok(0,":="))
	{
		cmd += strspn( cmd, "-/" );
		if (str_in( cmd, "l", "ls", "list", 0 ))
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

