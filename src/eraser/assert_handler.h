
# ifndef ASSERT_HANDLER_H
# define ASSERT_HANDLER_H


# define BOOST_ENABLE_ASSERT_HANDLER
# include <boost/assert.hpp>
# include <execinfo.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>

namespace eraser
{
	inline void backtrace()  __attribute((always_inline));
	inline void backtrace()
	{
		int bt_fd = ::open( "backtrace", O_RDWR|O_CREAT );
		const int max_size = 15;
		static void* stack_buffer[max_size];

		size_t size = ::backtrace( stack_buffer, max_size );
		::backtrace_symbols_fd( stack_buffer, size, bt_fd );
		::close( bt_fd );
	}

}


namespace boost
{
	inline void assertion_failed(char const * expr, char const * function, char const * file, long line)
	{
		eraser::backtrace();
		std::cerr << "^^^ ASSERTION FAILED '" << expr << "', in "  << function << ", " << file << ":" << line << "^^^ \n";
		::abort();
	}

	inline void assertion_failed_msg(char const * expr, char const * msg,
			char const * function, char const * file, long line)
	{
		eraser::backtrace();
		std::cerr << "^^^ ASSERTION FAILED '" << expr << "', in "  << function << ", " << file << ":" << line
				<< ", " << msg << "^^^ \n";
		::abort();
	}
}

# endif
