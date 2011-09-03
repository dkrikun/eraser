
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
inline void alarm( jclass cls, jvmti_traits::field_id_t field_id, const thread_t& thread )
{
	std::cerr
		<< "ALARM! " << "\n"
		<< "\tin class " << agent::instance()->class_sig( cls ) << "\n"
		<< "\tfield id " << field_id << "\n"
		<< "\tthread " << thread << "\n"
		;
}



// keeps shared var logic for each field
// only fields declared in the object class, not inherited ones

struct object_data : boost::noncopyable
{
		typedef jvmti_traits::field_id_t                     fields_key_t;
		//typedef std::map< fields_key_t, shared_var_t >		 map_t;
		typedef std::vector<shared_var_t*>					 map_t;
		map_t vars_;
		jobject obj_;
		std::string type_;

		object_data( jobject obj
				, jclass cls, fields_key_t* fields
				, size_t num_fields
				, shared_var_t::alarm_func_t alarm_func )
				: obj_( obj )
				, type_( "object_data" )
		{
			jint ret;
			for( size_t j=0; j<num_fields; ++j )
			{
				// skip static fields
				agent::instance()->jvmti()->GetFieldModifiers( cls, fields[j], &ret );
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
	shared_var_t::alarm_func_t f = boost::bind( &eraser::alarm, cls, _1, _2 );
	tag_object( obj, new object_data( obj, cls, field_ids, num_fields, f ));
}

inline shared_var_t* get_shared_var( jobject field_object, jfieldID field_id )
{
	object_data* od = get_tag<object_data>(field_object);
	BOOST_ASSERT( od != 0 );
	logger::instance()->level(5) << "OBJ TYPE=" << od->type_ << std::endl;
	JNIEnv* jni = agent::instance()->jni();
	BOOST_ASSERT( jni->IsSameObject( field_object, od->obj_) == JNI_TRUE );
	shared_var_t* res =  od->get_shared_var( field_id );
	BOOST_ASSERT_MSG( res, "FAILED search in shared_vars" );
	return res;
}

}

#endif
