

#include <jvmti.h>
#include "eraser/assert_handler.h"
#include "eraser/threads_manage.h"
#include "eraser/tag.h"
#include "eraser/agent.h"
#include "eraser/logger.h"

namespace eraser
{

thread_t* get_thread( jthread thread_id )
{
	thread_t* thread = get_tag<thread_t>( thread_id );
	if( thread != 0 )
	{
		logger::instance()->level(0) << "THREAD TYPE=" << thread->type_ << std::endl;
		BOOST_ASSERT( agent::instance()->jni()->IsSameObject( thread_id, thread->thread_id_ ) == JNI_TRUE );
	}
	return thread;
}

}
