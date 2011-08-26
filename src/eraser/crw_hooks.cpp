

#include <iostream>
#include <boost/xpressive/xpressive.hpp>

#include "eraser/crw_hooks.h"
#include "eraser/vm_hooks.h"
#include "eraser/agent.h"
#include "eraser/shared_vars_manage.h"
#include "eraser/shared_var.h"
#include "eraser/tag.h"
#include "eraser/thread.h"
#include "eraser/threads_manage.h"
#include "eraser/traits.h"
#include "eraser/logger.h"
#include "eraser/monitor.h"
#include <boost/array.hpp>

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
        std::string thread_name = agent::instance()->thread_name( thread_id );

        // filter out
        xpr::sregex filter = xpr::sregex::compile( "L" + eraser::agent::instance()->filter_regex_ + ";" );
        if( !xpr::regex_match( cls_sig, filter ) )
        	return;


		std::string name = agent::instance()->thread_name( thread_id );
        thread_t* thread = get_thread( thread_id );
        logger::instance()->level(1) << "NEW OBJ"
			<< "\n\t" << "jthread name= " << thread_name
			<< "\n\t" << "thread_t= " << thread << " " << *thread
			<< "\n\t" << "class sig= " << cls_sig
			<< std::endl;

        // get fields declared within the class of the object being created
        jint field_count = 0;
        jfieldID* fields;
        err = jvmti->GetClassFields( cls, &field_count, &fields );
        check_jvmti_error(jvmti, err, "get class fields");

        for( size_t j=0; j<field_count; ++j )
        	logger::instance()->level(1) << "\t\t" << j << fields[j] << "\n";

        // set up eraser logic for each field
//        jclass global_ref = jni->NewGlobalRef( cls );
        //BOOST_ASSERT( global_ref != 0 );

//        BOOST_ASSERT_MSG( field_count<=2 , "OOPS, to small array" );
//        shared_var_t** data = new shared_var_t*[2];
//        for( size_t j=0; j<field_count; ++j)
//        	data[j] = new shared_var_t( fields[j] );
//
//        debug_obj_data* x = new debug_obj_data;
//        x->field_id_ = fields[0];
//        x->obj_ = jni->NewGlobalRef(obj);
//        x->shared_var_ = new shared_var_t( fields[0] );
        init_object_data( obj, cls, fields, field_count );

        // set up read/write access watches
        // this also includes static variables (duplicated over all instances)
        // this is not clever, and anyway we cannot fetch the object tag from within
        // field access events, because there is no object!
        for( size_t j=0; j<field_count; ++j )
        {
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
        thread_t* thread = get_thread( thread_id );
        BOOST_ASSERT( thread );
        jobject global_ref = agent::instance()->jni()->NewWeakGlobalRef( obj );
        if( global_ref == 0 )
        		fatal_error("Out of memory while trying to create new global ref.");

		logger::instance()->level(1) << "MONITOR ENTER"
				<< "\n\t" << "jthread name= " << name
				<< "\n\t" << "thread_t= " << thread << " " << *thread
				<< "\n\t" << "monitor gl= " << global_ref
				<< std::endl;

        thread->lock( lock(global_ref) );

}
void native_monitor_exit(JNIEnv *jni, jclass klass, jthread thread_id, jobject obj)
{
		LOCK_AND_EXIT_ON_DEATH();
		std::string name = agent::instance()->thread_name( thread_id );
		thread_t* thread = get_thread( thread_id );
		BOOST_ASSERT( thread );

		logger::instance()->level(1) << "MONITOR EXIT"
				<< "\n\t" << "jthread_name= " << name
				<< "\n\t" << "thread_t= " << thread << " " << *thread
				<< "\n\t" << "monitor local= " << obj
				<< std::endl;

#		if 0
		// do we really need a global reference here?
		jobject global_ref = agent::instance()->jni()->NewWeakGlobalRef( obj );
        if( global_ref == 0 )
        			fatal_error("Out of memory while trying to create new global ref.");
#		endif
		thread->unlock( lock(obj) );
}

}
