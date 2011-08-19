
#ifndef AGENT_H
#define AGENT_H

#include <jvmti.h>
#include <boost/utility.hpp>
#include <boost/assert.hpp>
#include <boost/program_options.hpp>

// debug
#include <vector>
#include "sun/agent_util.h"
#include "eraser/logger.h"

namespace po = boost::program_options;

namespace eraser
{


struct agent : boost::noncopyable
{
        jvmtiEnv*                jvmti_;
        JNIEnv*                  jni_;
        jrawMonitorID            monitor_;
        bool                     death_active_;
        bool					 met_destroy_jvm_thread_;
        std::string 			 filter_regex_;

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
          	LOG_DEBUG( "thread state= " << thread_state );
        	jvmtiThreadInfo ti;
        	memset( &ti, 0, sizeof(ti) );
        	err = jvmti()->GetThreadInfo( thread, &ti );
        	check_jvmti_error( jvmti(), err, "get thread info" );
        	LOG_DEBUG( "thread name= " << ti.name
        			<< " priority= " << ti.priority
        			<< " is_daemon= " << (int)ti.is_daemon
        			<< " thread_group= " << ti.thread_group
        			<< " cls_loader= " << ti.context_class_loader );
        }

        void config( const std::string& args )
        {
        	po::options_description desc("Allowed options");

        	desc.add_options()
        	    ("help", "produce help message")
        	    ("filter", po::value< std::string >()->default_value(""), "regex to match classes for instrumentation")
        	;

        	po::variables_map vm;
        	std::vector<std::string> split_args = po::split_unix( args, "," );
        	po::store( po::command_line_parser(split_args).options(desc).run(), vm );
        	po::notify( vm );

        	if( vm.count("filter") )
        		filter_regex_ = vm["filter"].as<std::string>();
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
        		, met_destroy_jvm_thread_( false )
        {}
};



}

#endif
