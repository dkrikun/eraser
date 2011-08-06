

#include <jvmti.h>
#include "sun/agent_util.h"
#include "eraser/agent.h"
#include "eraser/monitor.h"
#include "eraser/instrumentation.h"
#include "eraser/vm_hooks.h"


// -------- Event callbacks -----------
# define ENABLE_EVENT_NOTIFICATION(event)\
        err = jvmti->SetEventNotificationMode( JVMTI_ENABLE, event, 0 );        \
        check_jvmti_error(jvmti, err, "set event notifications");

# define LOCK_AND_EXIT_ON_DEATH()\
        eraser::scoped_lock( jvmti, eraser::agent::instance()->monitor_ );      \
        if(eraser::agent::instance()->death_active_)                            \
                return;


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
        eraser::thread_start( jvmti, jni, thread_id );

        jvmtiError err;
        ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_VM_OBJECT_ALLOC );                                   
        ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_VM_DEATH );
        ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_FIELD_ACCESS );
        ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_FIELD_MODIFICATION );
        ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_THREAD_START );
        ENABLE_EVENT_NOTIFICATION( JVMTI_EVENT_THREAD_END );
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
        ERASER_LOG( eraser::agent::instance()->phase() );
        eraser::instrument_classfile( jvmti, jni, class_being_redefined, loader, name, protection_domain
                                , class_data_len, class_data, new_class_data_len, new_class_data );
}

void JNICALL vm_thread_start( jvmtiEnv *jvmti, JNIEnv *jni, jthread thread )
{
        LOCK_AND_EXIT_ON_DEATH();
        ERASER_LOG( eraser::agent::instance()->phase() );
        eraser::thread_start( jvmti, jni, thread );
}

void JNICALL vm_thread_end( jvmtiEnv *jvmti, JNIEnv *jni, jthread thread )
{
        LOCK_AND_EXIT_ON_DEATH();
        ERASER_LOG( eraser::agent::instance()->phase() );
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
        eraser::field_write( jvmti, jni, thread, method, location, field_klass, object, field, signature_type, new_value );
}

void JNICALL vm_death( jvmtiEnv *jvmti, JNIEnv *jni )
{
        eraser::scoped_lock( jvmti, eraser::agent::instance()->monitor_ );
        eraser::agent::instance()->death_active_ = true;
        eraser::disengage( jvmti, jni );
        
        // disable event callbacks
        jvmtiError err;
        jvmtiEventCallbacks callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        err = jvmti->SetEventCallbacks(&callbacks, (jint)sizeof(callbacks));
        check_jvmti_error(jvmti, err, "Cannot set jvmti callbacks");
}

// -------- OnLoad -----------

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
        jint rc;
        jvmtiError err;
        
        // init jvmti env
        jvmtiEnv* jvmti = 0;
        rc = vm->GetEnv( (void **)&jvmti, JVMTI_VERSION);
        if (rc != JNI_OK || jvmti == 0)
            fatal_error("ERROR: Unable to create jvmtiEnv, error=%d\n", rc);
        
        eraser::agent::instance()->jvmti_ = jvmti;
          
        // init agent lock
        err = jvmti->CreateRawMonitor("agent lock", &(eraser::agent::instance()->monitor_));
        check_jvmti_error(jvmti, err, "create raw monitor");
        
        // set capabilities
        jvmtiCapabilities   capabilities;
        memset(&capabilities, 0, sizeof(capabilities));
        capabilities.       can_tag_objects                                = 1;
        capabilities.       can_generate_garbage_collection_events         = 1;
        capabilities.       can_generate_all_class_hook_events             = 1;
        capabilities.       can_generate_object_free_events                = 1;
        capabilities.       can_get_source_file_name                       = 1;
        capabilities.       can_get_line_numbers                           = 1;
        capabilities.       can_generate_vm_object_alloc_events            = 1;
        capabilities.       can_generate_field_access_events               = 1;
        capabilities.       can_generate_field_modification_events         = 1;
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
Agent_OnUnload(JavaVM *vm) {}



