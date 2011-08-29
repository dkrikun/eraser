#ifndef LOCK_POLICY_H
#define LOCK_POLICY_H


#include "eraser/traits.h"
#include "eraser/agent.h"
#include "eraser/monitor.h"

namespace eraser
{

template <class Traits>
struct lock_policy
{
	void global_lock(){};
	void global_unlock(){};
};

//template <>
//struct lock_policy<jvmti_traits>
//{
//	void global_lock()
//	{
//		eraser::monitor_enter( agent::instance()->jvmti()
//				, eraser::agent::instance()->monitor_ );
//	}
//
//	void global_unlock()
//	{
//		eraser::monitor_exit( agent::instance()->jvmti()
//				, eraser::agent::instance()->monitor_ );
//	}
//};


}


#endif
