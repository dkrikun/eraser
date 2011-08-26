

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
		std::string thread_name = agent::instance()->thread_name( thread_id );
		if( ( thread_name == "DestroyJavaVM") || agent::instance()->met_destroy_jvm_thread_ )
		{
			agent::instance()->met_destroy_jvm_thread_ = true;
			agent::instance()->death_active_ = true;
			return;
		}

		jthread global_ref = agent::instance()->jni()->NewWeakGlobalRef( thread_id );
		if( global_ref == 0 )
			fatal_error("Out of memory while trying to create new global ref.");

		thread_t* thread = new thread_t( global_ref, thread_name );
		BOOST_ASSERT( thread != 0 );
		logger::instance()->level(1) <<  "THREAD START" << "\n"
				<< "\t" << "jthread gl= " << global_ref << "\n"
				<< "\t" << "jthread name= " << thread_name << "\n"
				<< "\t" << "thread_t= " << thread << " " << *thread << "\n"
				;

		tag_object( thread_id, thread );
}

void thread_end( jvmtiEnv *jvmti, JNIEnv *jni
                , jthread thread_id )
{
		std::string thread_name = agent::instance()->thread_name( thread_id );
		logger::instance()->level(1) << "THREAD END" << "\n"
			<< "\t" << "jthread name= " << thread_name << "\n"
			;

		thread_t* thread = get_thread( thread_id );
		if( thread == 0 )
			return;

		logger::instance()->level(1)
				<< "thread_t= " << thread << " " << *thread
				<< std::endl;

		clear_tag( thread_id );
}

void field_read( jvmtiEnv* jvmti, JNIEnv* jni
                  , jthread thread_id, jmethodID method
                  , jlocation location, jclass field_klass
                  , jobject object, jfieldID field )
{
	std::string thread_name = agent::instance()->thread_name( thread_id );
	std::string class_sig = agent::instance()->class_sig( field_klass );
	logger::instance()->level(1) << "FIELD READ" << "\n"
			<< "\t" << "class= " << class_sig	<< "\n"
			<< "\t" << "field= " << field	<< "\n"
			<< "\t" << "jthread name= " << thread_name << "\n"
			;

	if( object == 0 )	// static field
		return;

	thread_t* thread = get_thread( thread_id );
	BOOST_ASSERT( thread != 0 );
//	shared_var_t* shared_var = get_shared_var( object, field );
//	BOOST_ASSERT( shared_var != 0 );
	debug_obj_data* x = get_tag<debug_obj_data>( object );
	BOOST_ASSERT( jni->IsSameObject( x->obj_, object ) == JNI_TRUE );
	shared_var_t* shared_var = x->shared_var_;
	BOOST_ASSERT( shared_var != 0 );

	logger::instance()->level(1)
						<< "\t" << "thread_t= " << thread << " " << *thread
						<< std::endl;

	shared_var->read( *thread );
}

void field_write( jvmtiEnv* jvmti, JNIEnv* jni
                   , jthread thread_id, jmethodID method
                   , jlocation location, jclass field_klass
                   , jobject object, jfieldID field
                   , char signature_type, jvalue new_value )
{
	std::string thread_name = agent::instance()->thread_name( thread_id );
	std::string class_sig = agent::instance()->class_sig( field_klass );
	logger::instance()->level(1) << "FIELD WRITE" << "\n"
			<< "\t" << "class= " << class_sig	<< "\n"
			<< "\t" << "field= " << field	<< "\n"
			<< "\t" << "jthread name= " << thread_name	<< "\n"
			;

	if( object == 0 )	// static field
		return;

	thread_t* thread = get_thread( thread_id );
		BOOST_ASSERT( thread != 0 );
	//	shared_var_t* shared_var = get_shared_var( object, field );
	//	BOOST_ASSERT( shared_var != 0 );
		debug_obj_data* x = get_tag<debug_obj_data>( object );
		BOOST_ASSERT( jni->IsSameObject( x->obj_, object ) == JNI_TRUE );
		shared_var_t* shared_var = x->shared_var_;
		BOOST_ASSERT( shared_var != 0 );

	logger::instance()->level(1)
					<< "\t" << "thread_t= " << thread << " " << *thread
					<< std::endl;

	shared_var->write( *thread );
}

}
