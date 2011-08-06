#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include <jvmti.h>

namespace eraser
{

/*
 * Set up object & array creation hooks.
 */
void engage( jvmtiEnv* jvmti, JNIEnv* jni );

/*
 * Disable object & array creation hooks.
 */
void disengage( jvmtiEnv* jvmti, JNIEnv* jni );

/*
 * Instrument classfile to invoke new object & array creation hooks in <cinit> of java.lang.Object
 */
void instrument_classfile( jvmtiEnv* jvmti, JNIEnv* jni
                         , jclass class_being_redefined, jobject loader
                         , const char* name, jobject protection_domain
                         , jint class_data_len, const unsigned char* class_data
                         , jint* new_class_data_len, unsigned char** new_class_data );
}

#endif
