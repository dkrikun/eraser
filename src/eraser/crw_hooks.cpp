

#include <iostream>
#include <boost/xpressive/xpressive.hpp>

#include "eraser/crw_hooks.h"
#include "eraser/agent.h"
#include "eraser/shared_vars_manage.h"

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
        ERASER_LOG( cls_sig );

        // get fields declared within the class of the object being created
        jint field_count = 0;
        jfieldID* fields;
        err = jvmti->GetClassFields( cls, &field_count, &fields );
        check_jvmti_error(jvmti, err, "get class fields");

        // set up eraser logic for each field
        init_object_data( obj, cls, fields, (size_t)field_count );

        // set up read/write access watches
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

void native_method_entry(JNIEnv *jni, jclass klass, jthread thread, int cnum, int mnum)
{
  //    std::cerr << thread << ' '
  //            << cnum << ' '
  //            << mnum << ' '
  //            << std::endl;
}
void native_method_exit(JNIEnv *jni, jclass klass, jthread thread, int cnum, int mnum)
{
  //    std::cerr << thread << ' '
  //            << cnum << ' '
  //            << mnum << ' '
  //            << std::endl;
}

}
