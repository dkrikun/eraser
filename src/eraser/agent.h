
#ifndef AGENT_H
#define AGENT_H

#include <sstream>
#include <execinfo.h>
#include <jvmti.h>
#include <boost/utility.hpp>
#include "eraser/assert_handler.h"
#if defined( ERASER_DEBUG )
#include <vector>
#endif


# define LOCK_AND_EXIT_ON_DEATH()\
		do {																	\
        eraser::scoped_lock( eraser::agent::instance()->jvmti()					\
			, eraser::agent::instance()->monitor_ );     						\
        if(eraser::agent::instance()->death_active_)                            \
                return;															\
		} while(0)

namespace eraser
{

struct agent : boost::noncopyable
{
        jvmtiPhase phase() const;
        std::vector<jthread> all_threads() const;
        void dump_threads() const;
        void dump_threads( jthread to_compare );
        std::string class_sig( jclass cls ) const;
        std::string thread_name( jthread thread ) const;
        void reload_jni();

        static agent* instance()
        {
                static agent agent_;
                return &agent_;
        }



        JNIEnv* jni()
        {
                BOOST_ASSERT_MSG( jni_ != 0, "jni ptr has not been initialized" );
                reload_jni();
                return jni_;
        }

        jvmtiEnv* jvmti() const
        {
                BOOST_ASSERT_MSG( jvmti_ != 0, "jvmti ptr has not been initialized" );
                return jvmti_;
        }

        JavaVM* jvm() const
        {
        		BOOST_ASSERT_MSG( jvm_ != 0, "jvm ptr has not been initialized" );
        		return jvm_;
        }

        jvmtiEnv*                jvmti_;
        JNIEnv*                  jni_;
        JavaVM*				     jvm_;
        jrawMonitorID            monitor_;
        bool                     death_active_;
        bool					 met_destroy_jvm_thread_;
        std::string 			 filter_regex_;
        std::string 			 thread_filter_regex_;


   private:
        agent()
                : jvmti_(0)
				, jni_(0)
        		, jvm_(0)
        		, met_destroy_jvm_thread_( false )
        {}
};



}

#endif
