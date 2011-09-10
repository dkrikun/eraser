

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
        BOOST_ASSERT( jni != 0 );

        if( agent::instance()->phase() != JVMTI_PHASE_LIVE )
                return;

        jclass cls = agent::instance()->jni()->GetObjectClass(obj);
        std::string cls_sig = agent::instance()->class_sig( cls );
        std::string thread_name = agent::instance()->thread_name( thread_id );


        // filter out
        xpr::sregex filter = xpr::sregex::compile( agent::instance()->filter_regex_ );
        if( !xpr::regex_match( cls_sig, filter ) )
        	return;

#		if defined( ERASER_DEBUG )
		std::string name = agent::instance()->thread_name( thread_id );
        thread_t* thread = get_thread( thread_id );
        logger::instance()->level(1) << "NEW OBJ"
			<< "\n\t" << "jthread name= " << thread_name
			<< "\n\t" << "thread_t= " << thread << " " << *thread
			<< "\n\t" << "class sig= " << cls_sig
			<< std::endl;
#		endif

        // set up eraser logic for each field
        jobject obj_gr = agent::instance()->jni()->NewGlobalRef( obj );
        if( obj_gr == 0 )
        	fatal_error( "Insufficient memory for new ref" );
        jclass cls_gr = (jclass)agent::instance()->jni()->NewGlobalRef( cls );
        if( cls_gr == 0 )
        	fatal_error( "Insufficient memory for new ref" );

        std::vector<jfieldID> fields = agent::instance()->get_fields( cls );
        init_object_data( obj_gr, cls_gr, fields );

}

void native_newarr(JNIEnv *jni, jclass klass, jthread thread, jobject obj)
{
}

void native_monitor_enter(JNIEnv *jni, jclass klass, jthread thread_id, jobject obj)
{
		LOCK_AND_EXIT_ON_DEATH();
		std::string name = agent::instance()->thread_name( thread_id );
        thread_t* thread = get_thread( thread_id );
        if( thread == 0 )
        	return;
        jobject global_ref = agent::instance()->jni()->NewGlobalRef( obj );
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
        if( thread == 0 )
        	return;

		logger::instance()->level(1) << "MONITOR EXIT"
				<< "\n\t" << "jthread_name= " << name
				<< "\n\t" << "thread_t= " << thread << " " << *thread
				<< "\n\t" << "monitor local= " << obj
				<< std::endl;
		thread->unlock( lock(obj) );
}

}
