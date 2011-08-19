#ifndef LOGGER_H
#define LOGGER_H


#include <boost/utility.hpp>
#include <ostream>


namespace eraser
{

// logger, lower log level means higher importance

struct logger : boost::noncopyable
{
	static logger* instance()
	{
		static logger instance_;
		return &instance_;
	}

	void set_curr_level( unsigned int log_level )
	{
		curr_level_ = log_level;
	}

	unsigned int curr_level() const
	{
		return curr_level_;
	}

	std::ostream& level( unsigned int log_level )
	{
		return ( log_level <= curr_level_ )? log_to_ : null_sink_;
	}

	const static unsigned int ALWAYS = 0;
	const static unsigned int INFO = 1;
	const static unsigned int DEBUG = 2;

	struct null_sink_t : public std::ostream {};

	private:
	logger()
		: curr_level_(ALWAYS)
		, log_to_( std::cerr )
	{}
	unsigned int curr_level_;
	null_sink_t null_sink_;
	std::ostream& log_to_;
};

#define LOG_ERASER( msg, log_level )\
		eraser::logger::instance()->level( log_level )										\
				<< "[" << __FILE__ << "|" <<  __LINE__ << "|" << __FUNCTION__ << "] "	\
				<< msg																	\
				<< std::endl;

#define LOG_ALWAYS( msg ) LOG_ERASER( msg, eraser::logger::ALWAYS );
#define LOG_INFO( msg ) LOG_ERASER( msg, eraser::logger::INFO );
#define LOG_DEBUG( msg ) LOG_ERASER( msg, eraser::logger::DEBUG );

}

#endif
