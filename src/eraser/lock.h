#ifndef LOCK_H
#define LOCK_H

#include <jvmti.h>
#include <iostream>
#include <boost/operators.hpp>
#include <boost/concept_check.hpp>

#include "eraser/agent.h"
#include "eraser/logger.h"

namespace eraser
{

struct lock : boost::operators<lock>
{
	jobject lock_id_;

	explicit lock( jobject lock_id )
		: lock_id_( lock_id )
	{}

	// using default copy-ctor & assignment;
	bool operator==( const lock& rhs ) const
	{
		bool res = agent::instance()->jni()->IsSameObject( lock_id_, rhs.lock_id_ );
		LOG_INFO( "lock compare res: " << std::boolalpha << res );
		return res;
	}
};
BOOST_CONCEPT_ASSERT((boost::Assignable<lock>));
BOOST_CONCEPT_ASSERT((boost::EqualityComparable<lock>));

inline std::size_t hash_value( const lock& l )
{
	jvmtiError err;
	jint hash_code;
	err = agent::instance()->jvmti()->GetObjectHashCode( l.lock_id_, &hash_code );
	check_jvmti_error(agent::instance()->jvmti(), err, "get object hash code");
	std::size_t res = static_cast<std::size_t>(hash_code);
	LOG_INFO( "lock hash=" << res );
	return res;
}

inline std::ostream& operator<<( std::ostream& os, const lock& l )
{
	os << l.lock_id_;
	return os;
}

};

#endif
