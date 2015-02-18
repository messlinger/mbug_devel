
#ifndef MBUG_2811_HPP
#define MBUG_2811_HPP

//------------------------------------------------------------------------------

/** MBUG Library
	Class wrapper for
	2811 Thermometer device interface
*/

//------------------------------------------------------------------------------

#include "mbug.hpp"
#include "mbug_2811.h"

namespace mbug {

//------------------------------------------------------------------------------

class mbug_2811
{
	public:
		static device_list list( void )
			{
				return get_device_list(2811);
			}

		mbug_2811( int serial_num = 0 )
			{
				dev = mbug_2811_open(serial_num);
				if (!dev) throw mbug::error("mbug_2811: Error opening device.");
			}

		mbug_2811( char* id )
			{
				dev = mbug_2811_open_str(id);
				if (!dev) throw mbug::error("mbug_2811: Error opening device.");
			}

		~mbug_2811()
			{
				close();
			}

		void close( void )
			{
				mbug_2811_close(dev);
				dev = 0;
			}

		double read( void )
			{
				return mbug_2811_read(dev);
			}

		int set_mode(mbug_2811_mode mode)
			{
				return mbug_2811_set_mode( dev, mode );
			}

	protected:
		mbug_device dev;
};

//------------------------------------------------------------------------------

} // close namespace mbug

#endif // MBUG_2811_HPP