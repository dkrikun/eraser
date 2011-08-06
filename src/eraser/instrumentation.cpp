
#include <boost/utility.hpp>
#include "eraser/instrumentation.h"
#include "eraser/agent.h"
#include "eraser/crw_hooks.h"
#include "sun/agent_util.h"
#include "sun/java_crw_demo.h"

namespace eraser
{

// debugging, ignore
void mnr( unsigned ccount, const char** method_names
		, const char** method_signatures, int method_count )
{
	std::cerr << ccount;
	for( size_t j=0; j<method_count; ++j )
		std::cerr << "  " << method_names[j];
	for( size_t j=0; j<method_count; ++j )
		std::cerr << "\n" << method_signatures[j];
	std::cerr << std::endl;
}

#       define PROXY_CLASS               "Proxy"
#       define ENGAGED_FIELD             "engaged"
#       define METHOD_ENTRY              "method_entry"
#       define METHOD_EXIT               "method_exit"
#       define NEW_OBJ_METHOD            "newobj"
#       define NEW_ARR_METHOD            "newarr"
#       define NATIVE_NEW_OBJ_METHOD     "newobj_"
#       define NATIVE_NEW_ARR_METHOD     "newarr_"
#       define NATIVE_METHOD_ENTRY       "method_entry_"
#       define NATIVE_METHOD_EXIT        "method_exit_"

        /*
         * Set up object & array creation hooks.
         * This is achieved by binding new object and new array native methods and setting the 'engaged' flag.
         * Refer to Proxy.java for details.
         */
        void engage( jvmtiEnv* jvmti, JNIEnv* jni )
        {
                jint rc;

                static JNINativeMethod registry[4] =
                {
                //    method name           method signature                           routine to bind
                    { NATIVE_NEW_OBJ_METHOD, "(Ljava/lang/Object;Ljava/lang/Object;)V", (void*)native_newobj },
                    { NATIVE_NEW_ARR_METHOD, "(Ljava/lang/Object;Ljava/lang/Object;)V", (void*)native_newarr },
                    { NATIVE_METHOD_ENTRY,   "(Ljava/lang/Object;II)V",                 (void*)native_method_entry },
                    { NATIVE_METHOD_EXIT,    "(Ljava/lang/Object;II)V",                 (void*)native_method_exit }
                };
                
                jclass klass = jni->FindClass(PROXY_CLASS);
                if( klass == 0 )
                    fatal_error("ERROR: JNI: Cannot find %s with FindClass\n", PROXY_CLASS);

                rc = jni->RegisterNatives(klass, registry, 4);
                if( rc != 0 )
                    fatal_error("ERROR: JNI: Cannot register natives for class %s\n", PROXY_CLASS);

                jfieldID field = jni->GetStaticFieldID(klass, ENGAGED_FIELD, "I");
                if( field == 0 )
                    fatal_error("ERROR: JNI: Cannot get field from %s\n", PROXY_CLASS);
                    
                jni->SetStaticIntField(klass, field, 1);       

        }

        /*
         * Disable object & array creation hooks.
         * This is achieved by zeroing the 'engaged' flag in the proxy class.
         * Refer to Proxy.java for details.
         */
        void disengage( jvmtiEnv* jvmti, JNIEnv* jni )
        {
                jclass klass = jni->FindClass(PROXY_CLASS);
                if( klass == 0 )
                    fatal_error("ERROR: JNI: Cannot find %s with FindClass\n", PROXY_CLASS);

                jfieldID field = jni->GetStaticFieldID(klass, ENGAGED_FIELD, "I");
                if( field == 0 )
                    fatal_error("ERROR: JNI: Cannot get field from %s\n", PROXY_CLASS);
         
                jni->SetStaticIntField(klass, field, 0);
        }

        // determine class name for instrumentation
        inline const char* get_classname( const char* name, const unsigned char* class_data, jint class_data_len )
        {
                const char* classname;
                if( name == 0 ){ 
                    classname = java_crw_demo_classname(class_data, class_data_len, 0);
                    if ( classname == 0 )
                        fatal_error("ERROR: No classname in classfile\n");
                } else {
                    classname = strdup(name);
                    if ( classname == 0 )
                        fatal_error("ERROR: Ran out of malloc() space\n");
                }
                return classname;
        }

        /*
         * Instrument classfile to invoke new object & array creation hooks in <cinit> of java.lang.Object
         */
        void instrument_classfile( jvmtiEnv* jvmti, JNIEnv* jni
                                 , jclass class_being_redefined, jobject loader
                                 , const char* name, jobject protection_domain
                                 , jint class_data_len, const unsigned char* class_data
                                 , jint* new_class_data_len, unsigned char** new_class_data )
        {
                static size_t classfile_index;

                // determine classname
                const char* classname = get_classname( name, class_data, class_data_len );

                // for debug, inst. only java.lang.Object for now
                if( strcmp(classname, "java/lang/Object") != 0 )
                {
                        free((void*)classname);
                        return;
                }

                // prevent self-instrumentation
                if( strcmp(classname, PROXY_CLASS) == 0 )
                { 
                        free((void*)classname);
                        return;
                }

                std::cerr << "in instrument_classfile " << classname << std::endl;
                
                unsigned char* new_image = 0;
                long new_length = 0;
                *new_class_data_len = 0;
                *new_class_data = 0;
                
                classfile_index++;

                bool is_system_class = ( agent::instance()->phase() < JVMTI_PHASE_START );
                
                // go instrument
                java_crw_demo( classfile_index, classname
                             , class_data, class_data_len
                             , is_system_class
                             , PROXY_CLASS
                             , "L" PROXY_CLASS ";"                        
                             , METHOD_ENTRY, "(II)V"
                             , METHOD_EXIT, "(II)V"
                             , NEW_OBJ_METHOD, "(Ljava/lang/Object;)V"
                             , NEW_ARR_METHOD, "(Ljava/lang/Object;)V"
                             , &new_image
                    	     , &new_length
                             , 0, 0 );
                
                
                // memcpy transformed classfile to jvmti allocated memory
                if( new_length > 0 )
                {
                    unsigned char *jvmtispace = (unsigned char *)allocate( jvmti, (jint)new_length );
                    memcpy( (void*)jvmtispace, (void*)new_image, new_length );
                
                    *new_class_data_len = new_length;
                    *new_class_data = jvmtispace;
                }
                

                if( new_image != 0 )
                    free(new_image); 
                free((void*)classname);
        }
#       undef PROXY_CLASS               
#       undef ENGAGED_FIELD             
#       undef NEW_OBJ_METHOD            
#       undef NEW_ARR_METHOD            
#       undef METHOD_ENTRY
#       undef METHOD_EXIT
#       undef NATIVE_NEW_OBJ_METHOD     
#       undef NATIVE_NEW_OBJ_METHOD     
#       undef NATIVE_METHOD_ENTRY
#       undef NATIVE_METHOD_EXIT 

}
