#ifndef LOGGER_H
#define LOGGER_H


#include <boost/utility.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include "eraser/monitor.h"


namespace eraser
{

// logger, higher log level means higher importance

struct logger : boost::noncopyable
{

	static logger* instance()
	{
		static logger instance_;
		return &instance_;
	}


	struct null_sink_t : public std::ostream {};

	std::ostream& level( unsigned int log_level )
	{
		return ( log_level < curr_level_ )?
			null_sink_ : default_log_to_;
	}

	void set_curr_level( unsigned int log_level )
	{
		curr_level_ = log_level;
	}


	private:
	logger()
		: curr_level_(0)
		, default_log_to_( std::cerr )
	{}

	unsigned int curr_level_;
	null_sink_t null_sink_;
	std::ostream& default_log_to_;

	public:
	~logger(){}

};

#define INFO "[" << __FILE__ << "|" <<  __LINE__ << "|" << __FUNCTION__ << "]"

}

#endif
