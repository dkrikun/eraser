

#include <jvmti.h>
#include "eraser/agent.h"
#include "eraser/common.h"
#include "eraser/shared_vars_manage.h"


namespace eraser
{

// auxillary
const thread_t& get_thread( jthread thread_id )
{
		thread_t* od = get_tag<thread_t>( thread_id );
		BOOST_ASSERT( od != 0 );
		return *od;
}

// following are invoked from within vm event callbacks
void thread_start( jvmtiEnv *jvmti, JNIEnv *jni
                , jthread thread_id )
{
		jvmtiError err;

#		if 0
		ERASER_LOG( "thread_id=" << thread_id );
		agent::instance()->all_threads_dump();
		agent::instance()->dump_thread_debug( thread_id );
		ERASER_LOG( "thread_count=" << agent::instance()->all_threads().size() );
#		endif

		// debug, quick and dirty workaround for unclear sigsegv
		jvmtiThreadInfo ti;
		memset( &ti, 0, sizeof(ti) );
		err = agent::instance()->jvmti()->GetThreadInfo( thread_id, &ti );
		check_jvmti_error( agent::instance()->jvmti(), err, "get thread info" );
		if( strcmp( ti.name, "DestroyJavaVM") == 0 || agent::instance()->met_destroy_jvm_thread_ )
		{
			agent::instance()->met_destroy_jvm_thread_ = true;
			return;
		}

		jthread global_ref = agent::instance()->jni()->NewWeakGlobalRef( thread_id );
		ERASER_LOG( "global_ref=" << global_ref );
		if( global_ref == 0 )
			fatal_error("Out of memory while trying to create new global ref.");
		tag_object( thread_id, new thread_t( global_ref ) );
}

void thread_end( jvmtiEnv *jvmti, JNIEnv *jni
                , jthread thread_id )
{
		//ERASER_LOG( "thread_id=" << thread_id );
		thread_t* od = get_tag<thread_t>( thread_id );
		if( od == 0 )
			return;
		clear_tag( thread_id );
		ERASER_LOG( "global_ref=" << od->thread_id_ );
		//agent::instance()->jni()->DeleteGlobalRef( od->thread_id_ );
		delete od;
}

void field_read( jvmtiEnv* jvmti, JNIEnv* jni
                  , jthread thread_id, jmethodID method
                  , jlocation location, jclass field_klass
                  , jobject object, jfieldID field )
{
    	ERASER_LOG( "thread= " << thread_id
    		<< " object= " << object
    		<< " class= " << agent::instance()->class_sig( field_klass )
    		<< " field= " << field );

    	if( object == 0 )
    		return;

        thread_t thread = get_thread( thread_id );
        BOOST_ASSERT( agent::instance()->jni()->IsSameObject(thread.thread_id_,thread_id) );
        shared_var_t shared_var = get_shared_var( object, field );

        ERASER_LOG( "thread_t= " << thread );
        ERASER_LOG( "READ ");
        shared_var.read( thread );
}

void field_write( jvmtiEnv* jvmti, JNIEnv* jni
                   , jthread thread_id, jmethodID method
                   , jlocation location, jclass field_klass
                   , jobject object, jfieldID field
                   , char signature_type, jvalue new_value )
{
        ERASER_LOG( "thread= " << thread_id
        		<< " object= " << object
        		<< " class= " << agent::instance()->class_sig( field_klass )
        		<< " field= " << field );;

        if( object == 0)
        	return;

        thread_t thread         = get_thread( thread_id );
        BOOST_ASSERT( agent::instance()->jni()->IsSameObject(thread.thread_id_,thread_id) );
        shared_var_t shared_var = get_shared_var( object, field );

        ERASER_LOG( "thread_t= " << thread );
        ERASER_LOG( "WRITE ");
        shared_var.write( thread );
}

// these are not ever called yet because no built-in jvmti event exists
// that hooks on monitor enter/exit
// need to implement such traps by ourselves
void monitor_enter( jvmtiEnv* jvmti, JNIEnv* jni
                , jthread thread_id, lock_t lock )
{
        thread_t thread = get_thread( thread_id );
        thread.lock( lock );
}

void monitor_exit( jvmtiEnv* jvmti, JNIEnv* jni
                , jthread thread_id, lock_t lock )
{
        thread_t thread = get_thread( thread_id );
        thread.unlock( lock );
}

}
