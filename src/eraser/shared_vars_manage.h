
#ifndef SHARED_VARS_MANAGE_H
#define SHARED_VARS_MANAGE_H


#include <boost/xpressive/xpressive.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bind.hpp>

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

		boost::unordered_map< fields_key_t, shared_var_t > vars_;

		object_data( fields_key_t* fields
				, size_t num_fields
				, shared_var_t::alarm_func_t alarm_func )
		{
			for( size_t j=0; j<num_fields; ++j )
				vars_.insert( std::make_pair( fields[j], shared_var_t( fields[j], alarm_func ) ));
		}

		shared_var_t& get_shared_var( fields_key_t field )
		{
			return vars_.at( field );
		}

		const shared_var_t& get_shared_var( fields_key_t field ) const
		{
			return vars_.at( field );
		}
};

// sets up shared var logic for each field
// object class is only needed for reporting in alarm function
inline void init_object_data( jobject obj, jclass cls, jfieldID* field_ids, size_t num_fields )
{
	shared_var_t::alarm_func_t f = boost::bind( &eraser::alarm, cls, _1, _2 );
	tag_object( obj, new object_data( field_ids, num_fields, f ));
}

inline const shared_var_t& get_shared_var( jobject field_object, jfieldID field_id )
{
	 try
	 {
		 object_data* od = get_tag<object_data>(field_object);
		 BOOST_ASSERT( od != 0 );
		 return od->get_shared_var( field_id );
	 }
	 catch( const std::out_of_range& e )
	 {
		 std::cerr << "search in object shared vars failed" << std::endl;
		 throw;
	 }
}

//// --- set field watch ----
//inline void setup_field_watches( jclass cls )
//{
//	jvmtiError err;
//
//	// filter out all classes except for "inc" package
//	// nested packages are also filtered out
//	std::string cls_sig = agent::instance()->class_sig( cls );
//	xpr::sregex inc = xpr::as_xpr("Linc/") >> +xpr::alnum >> ';';
//	if( !xpr::regex_match( cls_sig, inc ) )
//		return;
//	ERASER_LOG( cls_sig );
//
//	// get class fields
//	jint field_count = 0;
//	jfieldID* fields;
//	err = agent::instance()->jvmti()->GetClassFields( cls, &field_count, &fields );
//	if( err != JVMTI_ERROR_NONE)
//	{
//		ERASER_LOG( "err=" << err );
//		return;
//	}
//	check_jvmti_error(agent::instance()->jvmti(), err, "get class fields");
//
//	// set read & write watches
//	for( size_t j=0; j<field_count; ++j )
//	{
//		err = agent::instance()->jvmti()->SetFieldAccessWatch( cls, fields[j] );
//		if( err != JVMTI_ERROR_NONE && err != JVMTI_ERROR_DUPLICATE )
//			check_jvmti_error(agent::instance()->jvmti(), err, "set field access watch");
//		err = agent::instance()->jvmti()->SetFieldModificationWatch( cls, fields[j] );
//		if( err != JVMTI_ERROR_NONE && err != JVMTI_ERROR_DUPLICATE )
//			check_jvmti_error(agent::instance()->jvmti(), err, "set field modification watch");
//	}
//}

}

#endif
