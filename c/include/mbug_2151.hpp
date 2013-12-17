
#ifndef MBUG_2151_HPP
#define MBUG_2151_HPP

//------------------------------------------------------------------------------

/** MBUG Library
	  Class wrapper for 
	  MBUG-2165 433 MHz Transmitter

      Stephan Messlinger
      Last change: 2013-08-04
*/

//------------------------------------------------------------------------------

#include "mbug.hpp"
#include "mbug_2151.h"

namespace mbug {

//------------------------------------------------------------------------------

class mbug_2151
{
	public:
		static device_list list( void )
			{
				return get_device_list(2151);
			}

		mbug_2151( int serial_num = 0 )
			{
				dev = mbug_2151_open(serial_num);
				if (!dev) throw mbug::error("mbug_2151: Error opening device.");
			}

		mbug_2151( char* id )
			{
				dev = mbug_2151_open_str(id);
				if (!dev) throw mbug::error("mbug_2151: Error opening device.");
			}
		
		~mbug_2151()
			{
				close();
			}
		
		void close( void )
			{
				mbug_2151_close(dev);
				dev = 0;
			}

		void set_sequence( unsigned char* data, size_t nbytes, mbug_2151_tx_mode tx_mode )
			{
				int ret = mbug_2151_set_sequence( dev, data, nbytes, tx_mode );
				if (ret<0) throw mbug::error("mbug_2151: Error in set_sequence.");
			}

		void set_sequence( std::vector<unsigned short> sequence )
			{
				// Unsigned short data buffer indicates 16-bit timed mode
				// NOTE: We may pass a pointer to the first sequence element, since vector elements
				//       are guaranteed to be stored contiguously in memory.
				int ret = mbug_2151_set_sequence( dev, (unsigned char*)&sequence[0], 
					                              2*sequence.size(), TX_MODE_TIMED_16 );
				if (ret<0) throw mbug::error("mbug_2151: Error in set_sequence.");
			}

		void set_seq_bitstream( std::vector<unsigned char> sequence )
			{
				// NOTE: We may pass a pointer to the first sequence element, since vector elements
				//       are guaranteed to be stored contiguously in memory.
				int ret = mbug_2151_set_sequence( dev, &sequence[0], sequence.size(), TX_MODE_BITSTREAM );
				if (ret<0) throw mbug::error("mbug_2151: Error in set_sequence.");			
			}

		void set_seq_times_8bit( std::vector<unsigned char> sequence )
			{
				// NOTE: We may pass a pointer to the first sequence element, since vector elements
				//       are guaranteed to be stored contiguously in memory.
				int ret = mbug_2151_set_sequence( dev, &sequence[0], sequence.size(), TX_MODE_TIMED_8 );
				if (ret<0) throw mbug::error("mbug_2151: Error in set_sequence.");
			}

		void set_seq_times_16bit( std::vector<unsigned short> sequence )
			{
				// NOTE: We may pass a pointer to the first sequence element, since vector elements
				//       are guaranteed to be stored contiguously in memory.
				int ret = mbug_2151_set_sequence( dev, (unsigned char*)&sequence[0], 
					                              2*sequence.size(), TX_MODE_TIMED_16 );
				if (ret<0) throw mbug::error("mbug_2151: Error in set_sequence.");
			}
		
		int get_seq_length( void )
			{
				int ret = mbug_2151_get_seq_length( dev );
				if (ret<0) throw mbug::error("mbug_2151: Error in get_sequence_length.");
				return ret;
			}

		void set_iterations( unsigned int niter )
			{
				int ret = mbug_2151_set_iterations( dev, niter );
				if (ret<0) throw mbug::error("mbug_2151: Error in set_iterations.");
			}

		int get_iterations( void )
			{		
				int ret = mbug_2151_get_iterations( dev );
				if (ret<0) throw mbug::error("mbug_2151: Error in get_iterations.");
				return ret;
			}

		int get_base_clock( void )
			{
				int ret = mbug_2151_get_base_clock( dev );
				if (ret<0) throw mbug::error("mbug_2151: Error in get_base_clock.");
				return ret;
			}

		void set_clock_div( unsigned int div )
			{
				int ret = mbug_2151_set_clock_div( dev, div );
				if (ret<0) throw mbug::error("mbug_2151: Error in set_clock_div.");
			}

		int get_clock_div( void )
			{
				int ret = mbug_2151_get_clock_div( dev );
				if (ret<0) throw mbug::error("mbug_2151: Error in get_clock_div.");
				return ret;
			}

		double set_bitrate( double freq )
			{
				double ret = mbug_2151_set_bitrate( dev, freq );
				if (ret<0.) throw mbug::error("mbug_2151: Error in set_bitrate.");
				return ret;
			}

		double set_interval( double interval )
			{
				double ret = mbug_2151_set_interval( dev, interval );
				if (ret<0.) throw mbug::error("mbug_2151: Error in set_interval.");
				return ret;
			}

		void start( void )
			{
				int ret = mbug_2151_start( dev );
				if (ret<0) throw mbug::error("mbug_2151: Error in start.");
			}
		
		void stop( bool force=0 )
			{
				int ret = mbug_2151_stop( dev, force );
				if (ret<0) throw mbug::error("mbug_2151: Error in stop.");
			}

		void stop_gracefully( void )
			{
				stop( 0 );
			}

		void stop_immediately( void )
			{
				stop( 1 );
			}

		void reset( void )
			{
				int ret = mbug_2151_reset( dev );
				if (ret<0) throw mbug::error("mbug_2151: Error in rest.");
			}

		int get_busy( void )
			{
				int ret = mbug_2151_reset( dev );
				if (ret<0) throw mbug::error("mbug_2151: Error in get_busy.");
				return ret;
			}

	protected:
		mbug_device dev;
};

//------------------------------------------------------------------------------

} // close namespace mbug

#endif // MBUG_2151_HPP