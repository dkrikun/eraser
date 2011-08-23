#ifndef LOGGER_H
#define LOGGER_H


#include <boost/utility.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include "eraser/monitor.h"


namespace eraser
{

// logger, lower log level means higher importance

struct logger : boost::noncopyable
{

	typedef std::string id_t;
	typedef std::map<id_t,std::ofstream*> logs_t;

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


	struct null_sink_t : public std::ostream {};
	std::ostream& level( unsigned int log_level )
	{
		return ( log_level <= curr_level_ )? default_log_to_ : null_sink_;
	}

	std::ostream& level( unsigned int log_level, id_t id )
	{
		logs_t::iterator it = logs_.find( id );
		BOOST_ASSERT_MSG( it != logs_.end(), "log not found" );
		if( log_level <= curr_level_ )
		{
			BOOST_ASSERT( it->second );
			return *(it->second);
		}
		else
			return null_sink_;
	}

	void add_log_file( const std::string& filename, id_t id )
	{
		logs_[id] = new std::ofstream( filename.c_str() );
		logs_[id]->setf(std::ios::unitbuf);
	}

	const static unsigned int ALWAYS = 0;
	const static unsigned int INFO = 1;
	const static unsigned int DEBUG = 2;

	private:
	logger()
		: curr_level_(ALWAYS)
		, default_log_to_( std::cerr )
	{}
	unsigned int curr_level_;
	null_sink_t null_sink_;
	std::ostream& default_log_to_;
	logs_t logs_;

	public:
	~logger()
	{
		for( logs_t::iterator it = logs_.begin(); it != logs_.end(); ++it )
		{
			std::ofstream* curr = it->second;
			if( curr != 0 )
				curr->close();
		}
	}
};

#define LOG_ERASER( msg, log_level )\
		do { 														\
		eraser::scoped_lock( eraser::agent::instance()->jvmti(),	\
				eraser::agent::instance()->monitor_ );				\
		eraser::logger::instance()->level( log_level )				\
				<< "[" << log_level <<  "|" << __FILE__ << "|"		\
				<<  __LINE__ << "|" << __FUNCTION__ << "] "			\
				<< msg												\
				<< std::endl;										\
		} while (0);

#define LOG_ALWAYS( msg, id ) LOG_ERASER( msg, eraser::logger::ALWAYS );
#define LOG_INFO( msg, id ) LOG_ERASER( msg, eraser::logger::INFO );
#define LOG_DEBUG( msg, id ) LOG_ERASER( msg, eraser::logger::DEBUG );


#define LOG_PER_THREAD( msg, log_level, id )\
		eraser::logger::instance()->level( log_level, id )			\
				<< "[" << log_level <<  "|" << __FILE__ << "|"		\
				<<  __LINE__ << "|" << __FUNCTION__ << "] "			\
				<< msg												\
				<< std::endl;

#define LOG_PT_ALWAYS( msg, id ) LOG_PER_THREAD( msg, eraser::logger::ALWAYS, id );
#define LOG_PT_INFO( msg, id ) LOG_PER_THREAD( msg, eraser::logger::INFO, id );
#define LOG_PT_DEBUG( msg, id ) LOG_PER_THREAD( msg, eraser::logger::DEBUG, id );

}

#endif
