

#include <jvmti.h>
#include "eraser/agent.h"
#include "eraser/common.h"
#include "eraser/shared_vars_manage.h"
#include "eraser/logger.h"
#include "eraser/threads_manage.h"



namespace eraser
{

// following are invoked from within vm event callbacks
void thread_start( jvmtiEnv *jvmti, JNIEnv *jni
                , jthread thread_id )
{
		// workaround, no new threads after 'DestroyJavaVM' are allowed
		if(( agent::instance()->thread_name( thread_id ) == "DestroyJavaVM")
				|| agent::instance()->met_destroy_jvm_thread_ )
		{
			agent::instance()->met_destroy_jvm_thread_ = true;
			agent::instance()->death_active_ = true;
			return;
		}

		std::string thread_name = agent::instance()->thread_name( thread_id );
		//logger::instance()->add_log_file( thread_name, thread_name );

		LOG_INFO( "THREAD START"
				<< " thread= " << thread_id
				<< " thread_name= " << thread_name
				, thread_name );


		jthread global_ref = agent::instance()->jni()->NewWeakGlobalRef( thread_id );
		LOG_DEBUG( "global_ref=" << global_ref, dummy );
		if( global_ref == 0 )
			fatal_error("Out of memory while trying to create new global ref.");
		thread_t* thread = new thread_t( global_ref );
		LOG_INFO( "thread_t= " << thread, thread_name );

		tag_object( thread_id, thread );
		thread_t* thread2 = get_thread(thread_id);
		LOG_INFO("A FUNNY LOG", "");

		//agent::instance()->dump_threads( thread_id );
}

void thread_end( jvmtiEnv *jvmti, JNIEnv *jni
                , jthread thread_id )
{
		std::string thread_name = agent::instance()->thread_name( thread_id );
		LOG_INFO( "THREAD END"
			<< " thread= " << thread_id
			<< " thread_name= " << thread_name
			, thread_name );

		thread_t* thread = get_thread( thread_id, "TE"+thread_name );
		if( thread == 0 )
			return;

		LOG_INFO( "thread_t= " << thread, thread_name );
		agent::instance()->dump_threads( thread_id );
		clear_tag( thread_id );
		delete thread;
}

void field_read( jvmtiEnv* jvmti, JNIEnv* jni
                  , jthread thread_id, jmethodID method
                  , jlocation location, jclass field_klass
                  , jobject object, jfieldID field )
{
		std::string thread_name = agent::instance()->thread_name( thread_id );
		LOG_INFO( "READ"
				<< " class= " << agent::instance()->class_sig( field_klass )
				<< " field= " << field
				, thread_name );

    	if( object == 0 )
    		return;

        thread_t* thread 		 = get_thread( thread_id, "FR "+thread_name );
        shared_var_t* shared_var = get_shared_var( object, field );

        LOG_INFO( "thread_t= " << *thread
        		<< " thread name= " << thread_name
        		, thread_name );

        agent::instance()->dump_threads( thread_id );
        shared_var->read( *thread );
}

void field_write( jvmtiEnv* jvmti, JNIEnv* jni
                   , jthread thread_id, jmethodID method
                   , jlocation location, jclass field_klass
                   , jobject object, jfieldID field
                   , char signature_type, jvalue new_value )
{
		std::string thread_name = agent::instance()->thread_name( thread_id );
		LOG_INFO( "WRITE"
			<< " class= " << agent::instance()->class_sig( field_klass )
			<< " field= " << field
			, thread_name );

        if( object == 0 )
        	return;

        thread_t* thread         = get_thread( thread_id, "FW "+thread_name );
        shared_var_t* shared_var = get_shared_var( object, field );

        LOG_INFO( "thread_t= " << *thread
        		<< " thread name= " << thread_name
        		, thread_name );

        agent::instance()->dump_threads( thread_id );
        shared_var->write( *thread );
}

}
