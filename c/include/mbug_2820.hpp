
#ifndef MBUG_2820_HPP
#define MBUG_2820_HPP

//------------------------------------------------------------------------------

/** MBUG Library
	Class wrapper for 
	2820 Environmeter device interface

     Stephan Messlinger
     Last change: 2013-12-15
*/

//------------------------------------------------------------------------------

#include <vector>

#include "mbug.hpp"
#include "mbug_2820.h"

namespace mbug {

//------------------------------------------------------------------------------

class mbug_2820
{
	public:
		static device_list list( void )
			{
				return get_device_list(2820);
			}

		mbug_2820( int serial_num = 0 )
			{
				dev = mbug_2820_open(serial_num);
				if (!dev) throw mbug::error("mbug_2820: Error opening device.");
			}

		mbug_2820( char* id )
			{
				dev = mbug_2820_open_str(id);
				if (!dev) throw mbug::error("mbug_2820: Error opening device.");
			}

		~mbug_2820()
			{
				close();
			}
		
		void close( void )
			{
				mbug_2820_close(dev);
				dev = 0;
			}

		std::vector<double> read( void )
			{
				std::vector<double> data(2);
				if (mbug_2820_read(dev, &data[0], &data[1]) < 0)
					throw mbug::error("mbug_2820: read error.");
				if (data[0]<=NOT_A_TEMPERATURE || data[1]<=NOT_A_TEMPERATURE)
					throw mbug::error("mbug_2820: sensor error.");
				return data;
			}

		unsigned long read_raw( void )
			{
				long data =  mbug_2820_read_raw(dev);
				if (data<0)
					throw mbug::error("mbug_2820: sensor or usb error.");
				return data;
			}
		
	protected:
		mbug_device dev;

};

//------------------------------------------------------------------------------

} // close namespace mbug

#endif // MBUG_2820_HPP