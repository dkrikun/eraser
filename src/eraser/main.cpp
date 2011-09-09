
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <jvmti.h>
#include "sun/agent_util.h"
#include "eraser/agent.h"
#include "eraser/logger.h"
#include "eraser/monitor.h"
#include "eraser/instrumentation.h"
#include "eraser/vm_hooks.h"


namespace po = boost::program_options;
namespace xpr = boost::xpressive;

// -------- Event callbacks -----------
# define ENABLE_EVENT_NOTIFICATION(event)\
        err = jvmti->SetEventNotificationMode( JVMTI_ENABLE, event, 0 );        \
        check_jvmti_error(jvmti, err, "set event notifications");


void JNICALL vm_start( jvmtiEnv *jvmti, JNIEnv *jni )
{
		eraser::scoped_lock( jvmti, eraser::agent::instance()->monitor_ );
			eraser::agent::instance()->death_active_ = false;

 		eraser::agent::instance()->jni_ = jni;
		eraser::engage( jvmti, jni );
}


void JNICALL vm_init( jvmtiEnv *jvmti, JNIEnv *jni, jthread thread_id )
{
	eraser::scoped_lock( jvmti, eraser::agent::instance()->monitor_ );
	eraser::agent::instance()->jni_ = jni;

	// emulate main thread start event
	//eraser::thread_start( jvmti, jni, thread_id );

	// set up r/w watches for already loaded classes
	eraser::logger::instance()->level(5) << "GOING TO SET UP WATCHES FOR LOADED CLASSES" << std::endl;
	std::vector<jclass> loaded_classes = eraser::agent::instance()->loaded_classes();
	for( std::vector<jclass>::iterator it = loaded_classes.begin(); it != loaded_classes.end(); ++ it )
		eraser::agent::instance()->setup_field_watches(*it);
	eraser::logger::instance()->level(5) << "FINISHED SETTING UP WATCHES FOR LOADED CLASSES" << std::endl;

	// enable additional required event notifications
	jvmtiError err;
	ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_VM_OBJECT_ALLOC );
	ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_VM_DEATH );
	ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_FIELD_ACCESS );
	ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_FIELD_MODIFICATION );
	ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_THREAD_START );
	ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_THREAD_END );
	//ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_CLASS_LOAD );
	ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_CLASS_PREPARE );
}


void JNICALL vm_object_alloc( jvmtiEnv *jvmti, JNIEnv *jni, jthread thread
               , jobject o, jclass object_klass, jlong size )
{
        std::cerr << "in object_alloc with thread_id " << thread << std::endl;
}

void JNICALL vm_classfile_load( jvmtiEnv *jvmti, JNIEnv* jni
                              , jclass class_being_redefined, jobject loader
                              , const char* name, jobject protection_domain
                              , jint class_data_len, const unsigned char* class_data
                              , jint* new_class_data_len, unsigned char** new_class_data )
{
        LOCK_AND_EXIT_ON_DEATH();
        eraser::agent::instance()->jni_ = jni;

        eraser::instrument_classfile( jvmti, jni, class_being_redefined, loader, name, protection_domain
                                , class_data_len, class_data, new_class_data_len, new_class_data );

}

void JNICALL vm_class_load( jvmtiEnv *jvmti, JNIEnv* jni, jthread thread, jclass cls )
{
	LOCK_AND_EXIT_ON_DEATH();
	eraser::agent::instance()->jni_ = jni;
	eraser::agent::instance()->setup_field_watches( cls );
}

void JNICALL vm_class_prepare( jvmtiEnv *jvmti, JNIEnv* jni, jthread thread, jclass cls )
{
	LOCK_AND_EXIT_ON_DEATH();
	eraser::agent::instance()->jni_ = jni;
	eraser::agent::instance()->setup_field_watches( cls );
}

void JNICALL vm_thread_start( jvmtiEnv *jvmti, JNIEnv *jni, jthread thread )
{
        LOCK_AND_EXIT_ON_DEATH();
        eraser::agent::instance()->jni_ = jni;
        //eraser::agent::instance()->jvm()->AttachCurrentThread((void**)&eraser::agent::instance()->jni_,0);
        eraser::thread_start( jvmti, jni, thread );
}

void JNICALL vm_thread_end( jvmtiEnv *jvmti, JNIEnv *jni, jthread thread )
{
        LOCK_AND_EXIT_ON_DEATH();
        eraser::agent::instance()->jni_ = jni;
        eraser::thread_end( jvmti, jni, thread );
}

void JNICALL vm_field_read( jvmtiEnv* jvmti, JNIEnv* jni
                          , jthread thread, jmethodID method
                          , jlocation location, jclass field_klass
                          , jobject object, jfieldID field )
{
        LOCK_AND_EXIT_ON_DEATH();
        BOOST_ASSERT( jni != 0 );
        BOOST_ASSERT( field != 0 );
        eraser::agent::instance()->jni_ = jni;
        eraser::field_read( jvmti, jni, thread, method, location, field_klass, object, field );
}

void JNICALL vm_field_write( jvmtiEnv* jvmti, JNIEnv* jni
                           , jthread thread, jmethodID method
                           , jlocation location, jclass field_klass
                           , jobject object, jfieldID field
                           , char signature_type, jvalue new_value )
{
        LOCK_AND_EXIT_ON_DEATH();
        BOOST_ASSERT( jni != 0 );
        BOOST_ASSERT( field != 0 );
        eraser::agent::instance()->jni_ = jni;
        eraser::field_write( jvmti, jni, thread, method, location
        		, field_klass, object, field, signature_type, new_value );
}

void JNICALL vm_death( jvmtiEnv *jvmti, JNIEnv *jni )
{
        eraser::scoped_lock( jvmti, eraser::agent::instance()->monitor_ );
        std::cerr << "in vm_death" << std::endl;
        eraser::agent::instance()->death_active_ = true;
        eraser::agent::instance()->jni_ = jni;
        eraser::disengage( jvmti, jni );
        
        // disable event callbacks
        jvmtiError err;
        jvmtiEventCallbacks callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        err = jvmti->SetEventCallbacks( &callbacks, (jint)sizeof(callbacks) );
        check_jvmti_error( jvmti, err, "Cannot set jvmti callbacks" );
}


// -------- OnLoad -----------

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
        jint rc;
        jvmtiError err;
        
        // init jvm env.
        eraser::agent::instance()->jvm_ = vm;

        // init jvmti env.
        jvmtiEnv* jvmti = 0;
        rc = vm->GetEnv( (void **)&jvmti, JVMTI_VERSION );
        if (rc != JNI_OK || jvmti == 0)
            fatal_error("ERROR: Unable to create jvmtiEnv, error=%d\n", rc);
        eraser::agent::instance()->jvmti_ = jvmti;


#		if 0
        char* log_level_envar = std::getenv("ERASER_LOG_LEVEL");
        unsigned int log_level = eraser::logger::DEBUG;
        if( log_level_envar != 0 )
        	log_level = boost::lexical_cast<unsigned int>( log_level_envar );
        eraser::logger::instance()->set_curr_level( log_level );
#		endif


        // init agent global mutex
        err = jvmti->CreateRawMonitor("agent lock", &(eraser::agent::instance()->monitor_));
        check_jvmti_error(jvmti, err, "create raw monitor");

        // parse command line options
        if( options != 0 )
        {
        	po::options_description desc("Allowed options");

        	unsigned int default_log_level = 1;
        	desc.add_options()
        	    ("help", "produce help message")
        	    ("log-level", po::value<unsigned int>()->default_value( default_log_level ), "log level" )
#				if defined( ERASER_DEBUG )
        	    ("obj-filter", po::value<std::string>()->default_value("inc\\.Cell")
        	    		, "regex to match classes (for field watches)")
				("thread-filter", po::value<std::string>()->default_value("inc\\.Worker"
						"|inc\\.SynchWorker"
						"|inc\\.MethodSynchWorker"
						"|inc\\.Cell"
						"|inc\\.CellInherit"
						)
						, "regex to match thread classes")
#				else
				("obj-filter", po::value<std::string>()->default_value("\\w+(\\w+|\\.)*\\w+")
						, "regex to match classes (for field watches)")
				("thread-filter", po::value<std::string>()->default_value("java.lang.Thread")
						, "regex to match thread classes")
#				endif
        	;

        	po::variables_map vm;
        	std::vector<std::string> split_args = po::split_unix( options, "," );
#			if defined( ERASER_DEBUG )
        	std::cerr << "Parsed args:\n\t";
        	std::copy( split_args.begin(), split_args.end(), std::ostream_iterator<std::string>(std::cerr," "));
        	std::cerr << std::endl;
#			endif
        	po::store( po::command_line_parser(split_args).options(desc).run(), vm );
        	po::notify( vm );

        	if( vm.count("help") )
        	{
        		std::cout << desc << std::endl;
        		return 0;
        	}

        	// replace dots for slashes
        	// i.e. java.lang.Object --> java/lang/Object
    		xpr::sregex dot = xpr::as_xpr("\\.");
    		std::string slash = "/";
        	eraser::agent::instance()
        		->filter_regex_ = xpr::regex_replace( vm["obj-filter"]
        		                                         .as<std::string>(), dot, slash );
        	eraser::agent::instance()
        		->thread_filter_regex_ = xpr::regex_replace( vm["thread-filter"]
        		                                                .as<std::string>(), dot, slash );

        	eraser::logger::instance()->set_curr_level( vm["log-level"].as<unsigned int>() );
        }
        eraser::logger::instance()->level(0) << "regex_filter= " << eraser::agent::instance()->filter_regex_ << std::endl;
        eraser::logger::instance()->level(0) << "regex_filter= " << eraser::agent::instance()->thread_filter_regex_ << std::endl;

        // set capabilities
        jvmtiCapabilities   capabilities;
        memset(&capabilities, 0, sizeof(capabilities));
        capabilities.       can_tag_objects                                = JNI_TRUE;
        capabilities.       can_generate_garbage_collection_events         = JNI_TRUE;
        capabilities.       can_generate_all_class_hook_events             = JNI_TRUE;
        capabilities.       can_generate_object_free_events                = JNI_TRUE;
        capabilities.       can_get_source_file_name                       = JNI_TRUE;
        capabilities.       can_get_line_numbers                           = JNI_TRUE;
        capabilities.       can_generate_vm_object_alloc_events            = JNI_TRUE;
        capabilities.       can_generate_field_access_events               = JNI_TRUE;
        capabilities.       can_generate_field_modification_events         = JNI_TRUE;
        err = jvmti->AddCapabilities(&capabilities);
        check_jvmti_error(jvmti, err, "add capabilities");
        
        
        // set up event callbacks
        jvmtiEventCallbacks callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.VMStart                 = &vm_start;
        callbacks.VMInit                  = &vm_init;
        callbacks.VMDeath                 = &vm_death;
        callbacks.ClassFileLoadHook       = &vm_classfile_load;
        callbacks.FieldAccess             = &vm_field_read;
        callbacks.FieldModification       = &vm_field_write;
        callbacks.ThreadStart             = &vm_thread_start;
        callbacks.ThreadEnd               = &vm_thread_end;
        callbacks.ClassPrepare			  = &vm_class_prepare;
        err = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
        check_jvmti_error(jvmti, err, "set event callbacks");
        
        // enable some event notifications
        ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_VM_START );                                   
        ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_VM_INIT );                                   
        // classfile hook event must be enabled here in order that we can instrument java.lang.Object.<init>
        ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_CLASS_FILE_LOAD_HOOK );                                   
        return 0;
}

JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM *vm)
{
	std::cerr << "Agent is being unloaded now" << std::endl;
}



