
#include <jvmti.h>
#include <vector>
#include <boost/xpressive/xpressive.hpp>
#include "eraser/assert_handler.h"
#include "eraser/logger.h"
#include "eraser/agent.h"
#include "eraser/threads_manage.h"
#include "sun/agent_util.h"

namespace xpr = boost::xpressive;


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
	jint ver = jni_->GetVersion();
	//logger::instance()->level(5000) << ver << std::endl;
	eraser::backtrace();
}

std::vector<jclass> agent::loaded_classes() const
{
	jvmtiError err;
	jint num_classes = 0;
	jclass* classes = 0;
	err = jvmti()->GetLoadedClasses( &num_classes, &classes );
	check_jvmti_error( jvmti(), err, "get loaded classes" );
	return std::vector<jclass>( classes, classes+num_classes );
}

std::vector<jfieldID> agent::get_fields(jclass cls) const
{
	 jvmtiError err;
	 jint field_count = 0;
	 jfieldID* fields = 0;
	 err = jvmti()->GetClassFields( cls, &field_count, &fields );
	 check_jvmti_error( jvmti(), err, "get class fields" );
	 // debug print
	 for( size_t j=0; j<field_count; ++j )
		 logger::instance()->level(1) << "\t\t" << j << " " << fields[j] << "\n";
	 return std::vector<jfieldID>( fields, fields+field_count );
}

void agent::setup_field_watches( jclass cls ) const
{
	jvmtiError err;
	jint cls_status = 0;
	err = jvmti()->GetClassStatus( cls, &cls_status );
	check_jvmti_error( jvmti(), err, "get class status" );

	logger::instance()->level(0) << "cls_status= " << std::hex << cls_status << std::endl;
	if( cls_status & JVMTI_CLASS_STATUS_ERROR )
		return;

	// filter out
	logger::instance()->level(0) << INFO << class_sig( cls ) << std::endl;
	xpr::sregex filter = xpr::sregex::compile( filter_regex_ );
	if( !xpr::regex_match( class_sig( cls ), filter ) )
		return;

	std::vector<jfieldID> fields = get_fields( cls );

	for( size_t j=0; j<fields.size(); ++j )
	{
		err = jvmti()->SetFieldAccessWatch( cls, fields[j] );
		check_jvmti_error( jvmti(), err, "set field access watch" );
		err = jvmti()->SetFieldModificationWatch( cls, fields[j] );
		check_jvmti_error( jvmti(), err, "set field modification watch" );
	}
}



}






