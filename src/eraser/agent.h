
#ifndef AGENT_H
#define AGENT_H

#include <jvmti.h>
#include <boost/utility.hpp>
#include <boost/assert.hpp>

#include "sun/agent_util.h"
#include "eraser/traits.h"

namespace eraser
{

# define ERASER_LOG(msg)\
	eraser::agent::instance()->logger_											\
		<< "[" << __FILE__ << "|" <<  __LINE__ << "|" << __FUNCTION__ << "] "	\
		<< msg																	\
		<< std::endl;

struct agent : boost::noncopyable
{
        jvmtiEnv*                jvmti_;
        JNIEnv*                  jni_;
        jrawMonitorID            monitor_;
        bool                     death_active_;
        std::ostream&			 logger_;

        jvmtiPhase phase() const
        {
                jvmtiError err;
                jvmtiPhase res;
                err = jvmti()->GetPhase(&res);
                check_jvmti_error( jvmti(), err, "get phase" );
                return res;
        }

        void all_threads_dump() const
        {
#				if defined( ERASER_DEBUG )
        		jvmtiError err;
        		jint threads_count = 0;
        		jthread* threads;
        		err = jvmti()->GetAllThreads( &threads_count, &threads );
        		check_jvmti_error( jvmti(), err, "get all threads" );
        		BOOST_ASSERT( threads != 0 );
        		for( size_t j=0; j<threads_count; ++j )
        			std::cerr << threads[j] << " ";
        		std::cerr << std::endl;
#				endif
        }

        static agent* instance()
        {
                static agent agent_;
                return &agent_;
        }

        JNIEnv* jni() const
        {
                BOOST_ASSERT_MSG( jni_ != 0, "jni ptr has not been initialized" );
                return jni_;
        }

        jvmtiEnv* jvmti() const
        {
                BOOST_ASSERT_MSG( jvmti_ != 0, "jvmti ptr has not been initialized" );
                return jvmti_;
        }

        std::string class_sig( jclass cls ) const
        {
        	jvmtiError err;
        	char* cls_sig;
        	err = jvmti()->GetClassSignature( cls, &cls_sig, 0 );
        	check_jvmti_error(jvmti(), err, "get class signature");
        	BOOST_ASSERT( cls_sig != 0 );
        	return cls_sig;
        }

   private:
        agent()
                : jvmti_(0)
				, jni_(0)
        		, logger_( std::cerr )
        {}
};



}

#endif
