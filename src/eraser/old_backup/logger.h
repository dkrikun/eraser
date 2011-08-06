
#ifndef LOGGER_H
#define LOGGER_H

namespace eraser
{

# define LOG(msg)\
	agent::instance()->logger_.log( __FILE__, __LINE__, __FUNCTION__, msg );

struct logger
{
		std::ostream& os_;

	    explicit logger( std::ostream& os )
	    	: os_( os )
	    {}

		void log( const std::string& file
        		, size_t line_num
        		, const std::string& function
        		, const std::string& message )
        {
			os_
				<< "["
				<< file << "|"
				<< line_num << "|"
				<< function
				<< "] "
				<< message
				<< std::endl
			;
        }
};

}

#endif
