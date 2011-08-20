#ifndef LOCK_H
#define LOCK_H

#include <jvmti.h>
#include <iostream>
#include <boost/operators.hpp>
#include <boost/concept_check.hpp>

namespace eraser
{

struct lock : boost::operators<lock>
{
	jobject lock_id_;

	explicit lock( jobject lock_id )
		: lock_id_( lock_id )
	{}

	// using default copy-ctor & assignment;
	bool operator==( const lock& rhs ) const;
};
BOOST_CONCEPT_ASSERT((boost::Assignable<lock>));
BOOST_CONCEPT_ASSERT((boost::EqualityComparable<lock>));

inline std::size_t hash_value( const lock& );

inline std::ostream& operator<<( std::ostream& os, const lock& l )
{
	os << l.lock_id_;
	return os;
}

};

#endif
