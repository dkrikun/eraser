#ifndef TAG_H
#define TAG_H

#include <jvmti.h>
#include "eraser/agent.h"
#include "sun/agent_util.h"
#include "eraser/logger.h"

namespace eraser
{

// faciliate arbitrate date attachment to jobject
template<class Data>
void tag_object( jobject obj, Data* data )
{
	jvmtiError err;
	jlong tag = (jlong)(void*)(ptrdiff_t)(data);
	err = agent::instance()->jvmti()->SetTag( obj, tag );
	check_jvmti_error( agent::instance()->jvmti(), err, "set tag" );
}

inline void clear_tag( jobject obj )
{
	jvmtiError err;
	err = agent::instance()->jvmti()->SetTag( obj, 0 );
	check_jvmti_error( agent::instance()->jvmti(), err, "set tag" );
}

template<class Data>
Data* get_tag( jobject obj )
{
	 jvmtiError err;
	 jlong tag;
	 err = agent::instance()->jvmti()->GetTag( obj, &tag );
	 check_jvmti_error( agent::instance()->jvmti(), err, "get tag" );
	 return (Data*)(void*)(ptrdiff_t)(tag);
}

}



#endif
