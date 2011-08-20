
#ifndef AGENT_H
#define AGENT_H

#include <jvmti.h>
#include <boost/utility.hpp>
#include <boost/assert.hpp>
#if defined( ERASER_DEBUG )
#include <vector>
#endif


# define LOCK_AND_EXIT_ON_DEATH()\
        eraser::scoped_lock( eraser::agent::instance()->jvmti()					\
			, eraser::agent::instance()->monitor_ );     						\
        if(eraser::agent::instance()->death_active_)                            \
                return;

namespace eraser
{

struct agent : boost::noncopyable
{
        jvmtiPhase phase() const;
        std::vector<jthread> all_threads() const;
        void dump_threads() const;
        void dump_threads( jthread to_compare ) const;
        std::string class_sig( jclass cls ) const;
        std::string thread_name( jthread thread ) const;


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

        jvmtiEnv*                jvmti_;
        JNIEnv*                  jni_;
        jrawMonitorID            monitor_;
        bool                     death_active_;
        bool					 met_destroy_jvm_thread_;
        std::string 			 filter_regex_;


   private:
        agent()
                : jvmti_(0)
				, jni_(0)
        		, met_destroy_jvm_thread_( false )
        {}
};



}

#endif
