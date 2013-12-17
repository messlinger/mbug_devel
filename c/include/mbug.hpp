
#ifndef MBUG_HPP
#define MBUG_HPP

//------------------------------------------------------------------------------

/** MBUG Library
	C++ wrapper

     Stephan Messlinger
     Last change: 2011-10-07
*/

//------------------------------------------------------------------------------

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

#include "mbug.h"

namespace mbug {

//------------------------------------------------------------------------------

	typedef std::vector<std::string> device_list;

	device_list get_device_list( int device_type = 0)
	{
		mbug_device_list p = 
			mbug_get_device_list(0);

		device_list list;
		for (int i=0; p[i]; i++)
		{
			list.push_back( std::string(p[i]) );
		}
		return list; 
	}

//------------------------------------------------------------------------------

	struct error: public std::runtime_error
	{
		error(const std::string& msg) 
			: runtime_error( msg + " " + mbug_error(0) ) 
			{
			}
	};

//------------------------------------------------------ ------------------------

} //Close namespace mbug

#endif //MBUG_HPP