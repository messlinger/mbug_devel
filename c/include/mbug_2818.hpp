
#ifndef MBUG_2818_HPP
#define MBUG_2818_HPP

//------------------------------------------------------------------------------

/** MBUG Library
	Class wrapper for
	2818 Thermometer device interface
*/

//------------------------------------------------------------------------------

#include <vector>

#include "mbug.hpp"
#include "mbug_2818.h"

namespace mbug {

//------------------------------------------------------------------------------

class mbug_2818
{
	public:
		static device_list list( void )
			{
				return get_device_list(2818);
			}

		mbug_2818( int serial_num = 0 )
			{
				dev = mbug_2818_open(serial_num);
				if (!dev) throw mbug::error("mbug_2818: Error opening device.");
			}

		mbug_2818( char* id )
			{
				dev = mbug_2818_open_str(id);
				if (!dev) throw mbug::error("mbug_2818: Error opening device.");
			}

		~mbug_2818()
			{
				close();
			}

		void close( void )
			{
				mbug_2818_close(dev);
				dev = 0;
			}

		double read( int channel )
			{
				return mbug_2818_read(dev, channel);
			}

		std::vector<double> read_all( void )
			{
				double atem[8];
				int ret = mbug_2818_read_all(dev, atem, 8);
				std::vector<double> vtem(atem, atem+8);
				return vtem;
			}

		std::vector<double> read_raw( void )
			{
				unsigned short dat[8];
				int ret = mbug_2818_read_raw(dev, dat, 8);
				std::vector<double> vdat(dat, dat+8);
				return vdat;
			}

		int set_acq_mode(mbug_acq_mode mode)
			{
				return mbug_2818_set_acq_mode( dev, mode );
			}

	protected:
		mbug_device dev;

};

//------------------------------------------------------------------------------

} // close namespace mbug

#endif // MBUG_2818_HPP
