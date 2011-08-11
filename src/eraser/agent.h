
#ifndef AGENT_H
#define AGENT_H

#include <jvmti.h>
#include <boost/utility.hpp>
#include <boost/assert.hpp>

// debug
#include <vector>
#include "sun/agent_util.h"

namespace eraser
{

# if defined( ERASER_DEBUG )
# define ERASER_LOG(msg)\
	eraser::agent::instance()->logger_											\
		<< "[" << __FILE__ << "|" <<  __LINE__ << "|" << __FUNCTION__ << "] "	\
		<< msg																	\
		<< std::endl;
# else
# define ERASER_LOG(msg) //nop
# endif

struct agent : boost::noncopyable
{
        jvmtiEnv*                jvmti_;
        JNIEnv*                  jni_;
        jrawMonitorID            monitor_;
        bool                     death_active_;
        std::ostream&			 logger_;
        bool					 met_destroy_jvm_thread_;

        jvmtiPhase phase() const
        {
                jvmtiError err;
                jvmtiPhase res;
                err = jvmti()->GetPhase(&res);
                check_jvmti_error( jvmti(), err, "get phase" );
                return res;
        }

#		if defined( ERASER_DEBUG )
        void all_threads_dump() const
        {

        		jvmtiError err;
        		jint threads_count = 0;
        		jthread* threads;
        		err = jvmti()->GetAllThreads( &threads_count, &threads );
        		check_jvmti_error( jvmti(), err, "get all threads" );
        		BOOST_ASSERT( threads != 0 );
        		for( size_t j=0; j<threads_count; ++j )
        			std::cerr << threads[j] << " ";
        		std::cerr << std::endl;

        }

        std::vector<jthread> all_threads() const
		{
        	jvmtiError err;
        	jint threads_count = 0;
        	jthread* threads;
        	err = jvmti()->GetAllThreads( &threads_count, &threads );
        	check_jvmti_error( jvmti(), err, "get all threads" );
        	BOOST_ASSERT( threads != 0 );
        	return std::vector<jthread>(threads,threads+threads_count);
		}

        void dump_thread_debug( jthread thread )
        {
        	jvmtiError err;
        	jint thread_state;
        	err = jvmti()->GetThreadState( thread, &thread_state );
        	check_jvmti_error( jvmti(), err, "get thread state" );
        	ERASER_LOG( "thread state= " << thread_state );
        	jvmtiThreadInfo ti;
        	memset( &ti, 0, sizeof(ti) );
        	err = jvmti()->GetThreadInfo( thread, &ti );
        	check_jvmti_error( jvmti(), err, "get thread info" );
        	ERASER_LOG( "thread name= " << ti.name
        			<< " priority= " << ti.priority
        			<< " is_daemon= " << (int)ti.is_daemon
        			<< " thread_group= " << ti.thread_group
        			<< " cls_loader= " << ti.context_class_loader );

        }
#		endif



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
        		, met_destroy_jvm_thread_( false )
        {}
};



}

#endif
