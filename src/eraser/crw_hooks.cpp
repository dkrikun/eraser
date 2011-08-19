

#include <iostream>
#include <boost/xpressive/xpressive.hpp>

#include "eraser/crw_hooks.h"
#include "eraser/vm_hooks.h"
#include "eraser/agent.h"
#include "eraser/shared_vars_manage.h"
#include "eraser/thread.h"
#include "eraser/traits.h"
#include "eraser/logger.h"

namespace xpr = boost::xpressive;


namespace eraser
{

// hooks on object creation
void native_newobj( JNIEnv *jni, jclass tracker_class, jthread thread, jobject obj )
{
        jvmtiError err;
        jvmtiEnv* jvmti = agent::instance()->jvmti();
        BOOST_ASSERT( jni != 0 );
        if( agent::instance()->phase() != JVMTI_PHASE_LIVE )
                return;

        jclass cls = jni->GetObjectClass(obj);
        std::string cls_sig = agent::instance()->class_sig( cls );

        // debugging, filter out all classes except for package "inc"
        // nested packages are also filtered out here
        xpr::sregex inc = xpr::as_xpr("Linc/") >> +xpr::alnum >> ';';
        if( !xpr::regex_match( cls_sig, inc ) )
        	return;
        LOG_INFO( cls_sig );

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
        	LOG_INFO( "field " << j << " " << fields[j] );
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
		LOG_INFO( "MONITOR ENTER"
				<< " thread= " << thread_id
				<< " monitor= " << obj );

#		if defined ( ERASER_DEBUG )
		jthread global_refa = agent::instance()->jni()->NewGlobalRef( thread_id );
		if( global_refa == 0 )
			fatal_error("Out of memory while trying to create new global ref.");
        LOG_DEBUG( "SAME? " << std::boolalpha << agent::instance()->same_as_last_thread( thread_id ) );
        agent::instance()->update_last_thread( global_refa );

//		static jthread last_thread;
//		bool res = agent::instance()->jni()->IsSameObject( last_thread, thread_id );
//		ERASER_LOG( "SAME?" << std::boolalpha << res );
//		last_thread = agent::instance()->jni()->NewGlobalRef(thread_id);

        agent::instance()->dump_thread_debug( thread_id );
# 	    endif

        static thread_t* last(0);
        thread_t* thread = get_thread( thread_id );
        LOG_DEBUG( "SAME SAME??" << std::boolalpha << (last == thread) );
        last = thread;
        jobject global_ref = agent::instance()->jni()->NewWeakGlobalRef( obj );
        if( global_ref == 0 )
        			fatal_error("Out of memory while trying to create new global ref.");

        thread->lock( lock(global_ref) );

}
void native_monitor_exit(JNIEnv *jni, jclass klass, jthread thread_id, jobject obj)
{
		LOG_INFO( "MONITOR EXIT"
			<< " thread= " << thread_id
			<< " monitor= " << obj );

		thread_t* thread = get_thread( thread_id );
		// do we really need a global reference here?
		jobject global_ref = agent::instance()->jni()->NewWeakGlobalRef( obj );
        if( global_ref == 0 )
        			fatal_error("Out of memory while trying to create new global ref.");
		thread->unlock( lock(global_ref) );
}

}
