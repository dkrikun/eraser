
#ifndef CRW_HOOKS_H
#define CRW_HOOKS_H

#include <jvmti.h>

namespace eraser
{

typedef void (*native_new_method_t)(JNIEnv *, jclass, jthread, jobject);

void native_newobj(JNIEnv *jni, jclass klass, jthread thread, jobject o);
void native_newarr(JNIEnv *jni, jclass klass, jthread thread, jobject o);
void native_method_entry(JNIEnv *jni, jclass klass, jthread thread, int cnum, int mnum);
void native_method_exit(JNIEnv *jni, jclass klass, jthread thread, int cnum, int mnum);

}

#endif

