
#ifndef VM_HOOKS
#define VM_HOOKS

#include "eraser/common.h"


namespace eraser
{

thread_t* get_thread( jthread thread_id );
void thread_start( jvmtiEnv *jvmti, JNIEnv *jni, jthread thread );
void thread_end( jvmtiEnv *jvmti, JNIEnv *jni, jthread thread );

void field_read( jvmtiEnv* jvmti, JNIEnv* jni
                  , jthread thread, jmethodID method
                  , jlocation location, jclass field_klass
                  , jobject object, jfieldID field );

void field_write( jvmtiEnv* jvmti, JNIEnv* jni
                   , jthread thread, jmethodID method
                   , jlocation location, jclass field_klass
                   , jobject object, jfieldID field
                   , char signature_type, jvalue new_value );
}

#endif
