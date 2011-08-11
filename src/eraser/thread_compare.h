
#ifndef THREAD_COMPARE_H
#define THREAD_COMPARE_H


#include <eraser/agent.h>
#include <eraser/traits.h>

namespace eraser
{

template <class Thread>
struct thread_compare
{
	typedef Thread thread_id_t;
	static bool is_equal( const thread_id_t& lhs, const thread_id_t& rhs )
	{
		return lhs == rhs;
	}
};


template <>
struct thread_compare <jvmti_traits::thread_id_t>
{

	typedef jvmti_traits::thread_id_t thread_id_t;
	static bool is_equal( const thread_id_t& lhs, const thread_id_t& rhs )
	{
		bool res = agent::instance()->jni()->IsSameObject( lhs, rhs );
		ERASER_LOG("thread compare res: " << std::boolalpha<< res);
		return res;

	}
};



}

#endif
