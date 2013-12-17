
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "mbug_2110.h"

//---------------------------------------------------------------

int device_id = 0;

enum Action { Help, List, ReadGPIO, WriteGPIO, SetTRIS,
              ReadADC, SetPWM  } action = ReadGPIO;

//---------------------------------------------------------------

int str_in( const char* str, ... )
{
	va_list args;
	int match = 0;
	va_start(args, str);
	while(1) {
		const char* next = va_arg(args, const char*);
		if (next==0 || strlen(next)==0 || (match=!strncmp( str,next,strlen(next)) ))
			break;
	}
	va_end(args);
	return match;
}

void parse_options( int argc, char* argv[] )
{
	int i;
	for (i=1; i<argc; i++)
	{
		const char* ap = argv[i];
		while ( *ap=='-' || *ap=='/' )
			ap++;

		if (str_in( ap, "l", "list", 0 ))
			action = List;
		else if (str_in(ap, "h", "help", 0))
			action = Help;
		else if (str_in(ap, "d", "dev", "device", 0 ))
			{
				ap += strcspn( ap, ":=" );
				if (*ap != 0)
					device_id = atoi( ap+1 );
			}
		else {
				printf("#### Unknown command: %s", argv[i]);
				exit(1);
			}
	}

}

//---------------------------------------------------------------

int main( int argc, char* argv[] )
{
	mbug_device dev = 0;

	parse_options( argc, argv );

	if (action==List)
		{
			// List all attached devices
			int i;
			mbug_device_list list = mbug_get_device_list(2110);
			for (i=0; list[i]!=0; i++ ) {
				puts(list[i]); putchar('\n');
			}
			return 0;
		}

	// Open device
	dev = mbug_2110_open( device_id );
	if (dev ==0 ) {
		puts("Error opening device.");
		return 1;
	}

	mbug_2110_close(dev);
	return 0;
}

