

#include <jvmti.h>
#include "eraser/agent.h"
#include "eraser/common.h"
#include "eraser/shared_vars_manage.h"
#include "eraser/logger.h"


namespace eraser
{

// auxillary
thread_t* get_thread( jthread thread_id )
{
		thread_t* od = get_tag<thread_t>( thread_id );
		BOOST_ASSERT( od != 0 );
        BOOST_ASSERT( agent::instance()->jni()->IsSameObject( od->thread_id_, thread_id ));
		return od;
}

// following are invoked from within vm event callbacks
void thread_start( jvmtiEnv *jvmti, JNIEnv *jni
                , jthread thread_id )
{
		jvmtiError err;

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
		LOG_DEBUG( "global_ref=" << global_ref );
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
		LOG_DEBUG( "global_ref=" << od->thread_id_ );
		//agent::instance()->jni()->DeleteGlobalRef( od->thread_id_ );
		delete od;
}

void field_read( jvmtiEnv* jvmti, JNIEnv* jni
                  , jthread thread_id, jmethodID method
                  , jlocation location, jclass field_klass
                  , jobject object, jfieldID field )
{
		LOG_INFO( "READ");
    	LOG_INFO( "class= " << agent::instance()->class_sig( field_klass ) << " field= " << field );

    	if( object == 0 )
    		return;

        thread_t* thread 		 = get_thread( thread_id );
        shared_var_t* shared_var = get_shared_var( object, field );

        LOG_INFO( "thread_t= " << *thread );

        shared_var->read( *thread );
}

void field_write( jvmtiEnv* jvmti, JNIEnv* jni
                   , jthread thread_id, jmethodID method
                   , jlocation location, jclass field_klass
                   , jobject object, jfieldID field
                   , char signature_type, jvalue new_value )
{
		LOG_INFO( "WRITE");
		LOG_INFO( "class= " << agent::instance()->class_sig( field_klass ) << " field= " << field );

        if( object == 0)
        	return;

        thread_t* thread         = get_thread( thread_id );
        shared_var_t* shared_var = get_shared_var( object, field );

        LOG_INFO( "thread_t= " << *thread );
        shared_var->write( *thread );
}

}
