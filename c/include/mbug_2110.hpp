
#ifndef MBUG_2110_HPP
#define MBUG_2110_HPP

//------------------------------------------------------------------------------

/** MBUG Library
	Class wrapper for 
	2110 GPIO interface

     Stephan Messlinger
     Last change: 2013-02-27
*/

//------------------------------------------------------------------------------

#include "mbug.hpp"
#include "mbug_2110.h"

namespace mbug {
//------------------------------------------------------------------------------

class mbug_2110
{
	public:
		static device_list list( void )
			{
				return get_device_list(2110);
			}

		mbug_2110( int serial_num = 0 )
			{
				dev = mbug_2110_open(serial_num);
				if (!dev) throw mbug::error("mbug_2110: Error opening device.");
			}

		mbug_2110( char* id )
			{
				dev = mbug_2110_open_str(id);
				if (!dev) throw mbug::error("mbug_2110: Error opening device.");
			}

		~mbug_2110()
			{
				close();
			}
		
		void close( void )
			{
				mbug_2110_close(dev);
				dev = 0;
			}

		int get_gpio( void )
			{
				return mbug_2110_get_gpio( dev );
			}

		int set_gpio( unsigned int bits )
			{
				return mbug_2110_set_gpio( dev, bits );
			}

		int set_tris( unsigned int tris )
			{
				return mbug_2110_set_tris( dev, tris );
			}

		int enable_adc( unsigned int channels )
			{
				return mbug_2110_enable_adc( dev, channels );
			}

		typedef std::vector<unsigned short> adc_list;
		adc_list read_adc( void )
			{
				const unsigned short * data = mbug_2110_read_adc( dev );
				adc_list values(8);
				for (int ch=0; ch<8; ch++)
					values[ch] = data[ch];
				return values;
			}

		int enable_pwm( int channels )
			{
				return mbug_2110_enable_pwm( dev, channels );
			}

		int set_pwm( int duty )
			{
				return mbug_2110_set_pwm( dev, duty );
			}

	protected:
		mbug_device dev;
};

//------------------------------------------------------------------------------

} // close namespace mbug

#endif // MBUG_2110_HPP