
#ifndef SHARED_VARS_MANAGE_H
#define SHARED_VARS_MANAGE_H


#include <boost/xpressive/xpressive.hpp>
#include <boost/unordered_map.hpp>
#include <map>
#include <boost/bind.hpp>
#include <classfile_constants.h>

#include "eraser/agent.h"
#include "eraser/common.h"
#include "eraser/traits.h"
#include "eraser/tag.h"


namespace xpr = boost::xpressive;

namespace eraser
{

// this is invoked when eraser logic decides to fire a data race alarm
inline void alarm( int cls, jvmti_traits::field_id_t field_id, const thread_t& thread )
{
	std::cerr
		<< "ALARM! " << "\n"
		//<< "\tin class " << agent::instance()->class_sig( cls ) << "\n"
		<< "\tfield id " << field_id << "\n"
		<< "\tthread " << thread << "\n"
		;
}


struct debug_obj_data
{
	jobject obj_;
	jfieldID field_id_;
	shared_var_t* shared_var_;
};

// keeps shared var logic for each field
// only fields declared in the object class, not inherited ones
struct object_data : boost::noncopyable
{
		typedef jvmti_traits::field_id_t                     fields_key_t;
		//typedef std::map< fields_key_t, shared_var_t >		 map_t;
		typedef std::vector<shared_var_t*>					 map_t;
		map_t vars_;

		object_data( fields_key_t* fields
				, size_t num_fields, jclass cls
				, shared_var_t::alarm_func_t alarm_func )
		{
			jint ret;
			for( size_t j=0; j<num_fields; ++j )
			{
				// skip static fields
				agent::instance()->jvmti()->GetFieldModifiers(cls, fields[j], &ret);
				if (ret & JVM_ACC_STATIC)
					continue;
				//vars_.insert( std::make_pair( fields[j], shared_var_t( fields[j], alarm_func ) ));
				vars_.push_back( new shared_var_t( fields[j], alarm_func ) );
			}
		}

		shared_var_t* get_shared_var( fields_key_t field )
		{
			for( size_t j=0; j<vars_.size(); ++j )
				if( vars_[j]->field_id_	== field )
					return vars_[j];
			return 0;
		}
};

// sets up shared var logic for each field
// object class is only needed for reporting in alarm function
inline void init_object_data( jobject obj, jclass cls, jfieldID* field_ids, size_t num_fields )
{
	shared_var_t::alarm_func_t f = boost::bind( &eraser::alarm, 0, _1, _2 );
	tag_object( obj, new object_data( field_ids, num_fields, cls, f ));
}

inline shared_var_t* get_shared_var( jobject field_object, jfieldID field_id )
{
	shared_var_t** od = get_tag<shared_var_t*>(field_object);
	BOOST_ASSERT( od != 0 );
	//shared_var_t* res =  od->get_shared_var( field_id );
	BOOST_ASSERT( od[0] && od[1] );
	if( od[0]->field_id_ == field_id )
		return od[0];
	else if( od[1]->field_id_ == field_id )
		return od[1];
	else
		BOOST_ASSERT_MSG( false, "FAILED search in shared_vars" );
	return 0;
}

}

#endif
