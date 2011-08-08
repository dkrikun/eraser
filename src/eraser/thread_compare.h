
#ifndef THREAD_COMPARE_H
#define THREAD_COMPARE_H


#include <eraser/agent.h>
#include <eraser/traits.h>

namespace eraser
{

template <class EraserTraits>
struct thread_compare
{
	typedef typename EraserTraits::thread_id_t thread_id_t;
	bool is_same( const thread_id_t& lhs, const thread_id_t& rhs ) const
	{
		return lhs == rhs;
	}
};


template <>
struct thread_compare <jvmti_traits>
{

	typedef jvmti_traits::thread_id_t thread_id_t;
	bool is_same( const thread_id_t& lhs, const thread_id_t& rhs ) const
	{
		bool res = agent::instance()->jni()->IsSameObject( lhs, rhs );
		//ERASER_LOG("thread compare res: " << std::boolalpha<< res);
		return res;

	}
};

}

#endif
