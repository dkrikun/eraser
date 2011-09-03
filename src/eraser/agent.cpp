
#include <jvmti.h>
#include <vector>
#include "eraser/assert_handler.h"
#include "eraser/logger.h"
#include "eraser/agent.h"
#include "eraser/threads_manage.h"
#include "sun/agent_util.h"




namespace eraser
{

jvmtiPhase agent::phase() const
{
	jvmtiError err;
	jvmtiPhase res;
	err = jvmti()->GetPhase(&res);
	check_jvmti_error( jvmti(), err, "get phase" );
	return res;
}

std::vector<jthread> agent::all_threads() const
{
	jvmtiError err;
	if( phase() != JVMTI_PHASE_LIVE )
		return std::vector<jthread>();
	jint threads_count = 0;
	jthread* threads;
	err = jvmti()->GetAllThreads( &threads_count, &threads );
	check_jvmti_error( jvmti(), err, "get all threads" );
	BOOST_ASSERT( threads != 0 );
	return std::vector<jthread>(threads,threads+threads_count);
}


void agent::dump_threads() const
{
	logger::instance()->level(1) << " ALL THREADS DUMP \n";
	std::vector<jthread> threads = all_threads();
	for( size_t j=0; j<threads.size(); ++j )
	{
		jthread& curr = threads[j];
		logger::instance()->level(1)
							<< "\t" << "jthread=" << curr << "\n"
							<< "\t" << "thread name= " << thread_name( curr )
							<< std::endl;
	}
}

void agent::dump_threads( jthread to_compare )
{
	std::string name = thread_name(to_compare);
	logger::instance()->level(1) << " ALL THREADS DUMP compared to " << name;
	std::vector<jthread> threads = all_threads();
	for( size_t j=0; j<threads.size(); ++j )
	{
		jthread& curr = threads[j];
		logger::instance()->level(1)
				<< "\n\t" << "thread name= " << thread_name( curr )
				<< "\t" << std::boolalpha << ( jni()->IsSameObject( curr, to_compare ) == JNI_TRUE )
				<< std::endl;
	}
}

std::string agent::class_sig( jclass cls ) const
{
	jvmtiError err;
	char* cls_sig;
	err = jvmti()->GetClassSignature( cls, &cls_sig, 0 );
	check_jvmti_error(jvmti(), err, "get class signature");
	BOOST_ASSERT( cls_sig != 0 );
	return cls_sig;
}

std::string agent::thread_name( jthread thread ) const
{
	jvmtiError err;
	if( phase() != JVMTI_PHASE_LIVE )
		return "<not in the live phase, sorry>";
	jvmtiThreadInfo ti;
	memset( &ti, 0, sizeof(ti) );
	err = jvmti()->GetThreadInfo( thread, &ti );
	check_jvmti_error( jvmti(), err, "get thread info" );
	return ti.name;
}

void agent::reload_jni()
{
	jint res;
	JNIEnv* env = 0;
	res = jvm()->GetEnv( (void**)&env, JNI_VERSION_1_2 );
	if( res == JNI_EDETACHED )
	{
		res = jvm()->AttachCurrentThread( (void **)&env, 0 );
		if( res != JNI_OK || env == 0 )
			fatal_error( "ERROR: Unable to create JNIEnv by attach, error=%d\n", res );
	}
    else if( res != JNI_OK || env == 0 )
		fatal_error( "ERROR: Unable to create JNIEnv, error=%d\n", res );
	jni_ = env;
}


}






