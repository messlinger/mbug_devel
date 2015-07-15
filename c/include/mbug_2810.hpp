
#ifndef MBUG_2810_HPP
#define MBUG_2810_HPP

//------------------------------------------------------------------------------

/** MBUG Library
	Class wrapper for
	2810 Thermometer device interface
*/

//------------------------------------------------------------------------------

#include "mbug.hpp"
#include "mbug_2810.h"

namespace mbug {

//------------------------------------------------------------------------------

class mbug_2810
{
	public:
		static device_list list( void )
			{
				return get_device_list(2810);
			}

		mbug_2810( int serial_num = 0 )
			{
				dev = mbug_2810_open(serial_num);
				if (!dev) throw mbug::error("mbug_2810: Error opening device.");
			}

		mbug_2810( char* id )
			{
				dev = mbug_2810_open_str(id);
				if (!dev) throw mbug::error("mbug_2810: Error opening device.");
			}

		~mbug_2810()
			{
				close();
			}

		void close( void )
			{
				mbug_2810_close(dev);
				dev = 0;
			}

		double read( void )
			{
				return mbug_2810_read(dev);
			}

		int set_acq_mode(mbug_acq_mode mode)
			{
				return mbug_2810_set_acq_mode( dev, mode );
			}

	protected:
		mbug_device dev;
};

//------------------------------------------------------------------------------

} // close namespace mbug

#endif // MBUG_2810_HPP
