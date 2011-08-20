

#include <iostream>
#include <boost/xpressive/xpressive.hpp>

#include "eraser/crw_hooks.h"
#include "eraser/agent.h"
#include "eraser/shared_vars_manage.h"
#include "eraser/thread.h"
#include "eraser/threads_manage.h"
#include "eraser/traits.h"
#include "eraser/logger.h"
#include "eraser/monitor.h"

namespace xpr = boost::xpressive;


namespace eraser
{

// hooks on object creation
void native_newobj( JNIEnv *jni, jclass tracker_class, jthread thread_id, jobject obj )
{
		LOCK_AND_EXIT_ON_DEATH();
        jvmtiError err;
        jvmtiEnv* jvmti = agent::instance()->jvmti();
        BOOST_ASSERT( jni != 0 );
        if( agent::instance()->phase() != JVMTI_PHASE_LIVE )
                return;

        jclass cls = jni->GetObjectClass(obj);
        std::string cls_sig = agent::instance()->class_sig( cls );


#		if 0
        // debugging, filter out all classes except for package "inc"
        // nested packages are also filtered out here
        xpr::sregex inc = xpr::as_xpr("Linc/") >> +xpr::alnum >> ';';
#		endif
        // filter out
        xpr::sregex filter = xpr::sregex::compile( "L" + eraser::agent::instance()->filter_regex_ + ";" );
        if( !xpr::regex_match( cls_sig, filter ) )
        	return;


		std::string name = agent::instance()->thread_name( thread_id );
        thread_t* thread = get_thread( thread_id, "NNO"+name );
		LOG_INFO( "NEW OBJ"
			<< " thread= " << thread_id
			<< " thread_name= " << agent::instance()->thread_name( thread_id )
			<< " thread_t= " << thread
			<< " class sig= " << cls_sig
			, name
			);

		agent::instance()->dump_threads( thread_id );

        // get fields declared within the class of the object being created
        jint field_count = 0;
        jfieldID* fields;
        err = jvmti->GetClassFields( cls, &field_count, &fields );
        check_jvmti_error(jvmti, err, "get class fields");

        // set up eraser logic for each field
        init_object_data( obj, cls, fields, (size_t)field_count );

        // set up read/write access watches
        // this also includes static variables (duplicated over all instances)
        // this is not clever, and anyway we cannot fetch the object tag from within
        // field access events, because there is no object!
        for( size_t j=0; j<field_count; ++j )
        {
        	LOG_INFO( "field " << j << " " << fields[j], name );
        	err = jvmti->SetFieldAccessWatch( cls, fields[j] );
    		// rude, probably better manage duplicates by ourselves
        	if( err != JVMTI_ERROR_NONE && err != JVMTI_ERROR_DUPLICATE )
        		check_jvmti_error(jvmti, err, "set field access watch");
        	err = jvmti->SetFieldModificationWatch( cls, fields[j] );
        	if( err != JVMTI_ERROR_NONE && err != JVMTI_ERROR_DUPLICATE )
        		check_jvmti_error(jvmti, err, "set field modification watch");
        }
}

void native_newarr(JNIEnv *jni, jclass klass, jthread thread, jobject obj)
{
}

void native_monitor_enter(JNIEnv *jni, jclass klass, jthread thread_id, jobject obj)
{
		LOCK_AND_EXIT_ON_DEATH();
		std::string name = agent::instance()->thread_name( thread_id );
		LOG_INFO( "MONITOR ENTER"
				<< " thread= " << thread_id
				<< " thread_name= " << agent::instance()->thread_name( thread_id )
				<< " monitor= " << obj
				, name );

        thread_t* thread = get_thread( thread_id, "NMEN"+name );
        BOOST_ASSERT( thread );
        LOG_INFO( "thread_t= " << thread, name );
        jobject global_ref = agent::instance()->jni()->NewWeakGlobalRef( obj );
        if( global_ref == 0 )
        		fatal_error("Out of memory while trying to create new global ref.");

        agent::instance()->dump_threads( thread_id );
        thread->lock( lock(global_ref) );

}
void native_monitor_exit(JNIEnv *jni, jclass klass, jthread thread_id, jobject obj)
{
		LOCK_AND_EXIT_ON_DEATH();
		std::string name = agent::instance()->thread_name( thread_id );
		LOG_INFO( "MONITOR EXIT"
			<< " thread= " << thread_id
			<< " thread_name= " << agent::instance()->thread_name( thread_id )
			<< " monitor= " << obj
			, name );

		thread_t* thread = get_thread( thread_id, "NMEX"+name );
		BOOST_ASSERT( thread );
        LOG_INFO( "thread_t= " << thread , name );
#		if 0
		// do we really need a global reference here?
		jobject global_ref = agent::instance()->jni()->NewWeakGlobalRef( obj );
        if( global_ref == 0 )
        			fatal_error("Out of memory while trying to create new global ref.");
#		endif
        agent::instance()->dump_threads( thread_id );
		thread->unlock( lock(obj) );
}

}
