#ifndef FIELD_UNIQ_H
#define FIELD_UNIQ_H

#include <boost/operators.hpp>
#include <boost/functional/hash.hpp>
#include <jvmti.h>
#include <iostream>

namespace eraser
{

struct field_uniq : boost::operators< field_uniq >
{
        JNIEnv*  jni_;
        jvmtiEnv*  jvmti_;
        jobject  obj_;
        jfieldID field_;

        field_uniq( JNIEnv* jni
                  , jvmtiEnv* jvmti
                  , jobject obj
                  , jfieldID field )

                  : jni_( jni )
                  , jvmti_( jvmti )
                  , obj_( obj )
                  , field_( field )
        {
                BOOST_ASSERT_MSG( jni_ != 0, "jni ptr is null in field_uniq ctor" );
                BOOST_ASSERT_MSG( jvmti_ != 0, "jvmti ptr is null in field_uniq ctor" );
        }


        bool operator==( const field_uniq& rhs ) const
        {
                BOOST_ASSERT_MSG( jni_ != 0, "jni ptr is null in field_uniq operator==" );
                
                return ( jni_->IsSameObject( obj_, rhs.obj_ )
                                && ( field_ == rhs.field_ ) );
        }
};

inline std::ostream& operator<<( std::ostream& os, const field_uniq& k )
{
        os << "( " << k.obj_ << ", " << k.field_ << " )";
        return os;
}

inline std::size_t hash_value( const field_uniq&  k )
{
        std::size_t seed = 0;
        boost::hash_combine( seed, k.obj_ );
        boost::hash_combine( seed, k.field_ );
        return seed;
}

}

#endif

