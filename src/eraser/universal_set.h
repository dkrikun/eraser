
#ifndef ERASER_SET_H
#define ERASER_SET_H

#include <boost/optional/optional.hpp>
#include "eraser/assert_handler.h"
#include <boost/operators.hpp>
#include <boost/concept_check.hpp>
#include <list>
#include <iterator>


namespace eraser
{

        
template < class T >
struct universal_set : boost::operators< universal_set<T> >
{
        BOOST_CONCEPT_ASSERT((boost::Assignable<T>));
        BOOST_CONCEPT_ASSERT((boost::EqualityComparable<T>));

        private:
        typedef typename std::list<T>			      container_t;
        typedef typename container_t::iterator        iterator;
        typedef typename container_t::const_iterator  const_iterator;

        boost::optional< container_t > storage_;

        protected:
        universal_set( bool is_universal )
                : storage_( !is_universal, container_t() )            // storage_ is uninitialized iff the set is universal
        {}

        // default copy-ctor & assignment are used

        public:
        void intersect_with( const universal_set& rhs )
        {
                if( rhs.universal() )
                        return;

                if( universal() )
                {
                        storage_ = rhs.storage_;
                        return;
                }
                in_place_intersect( storage_.get(), rhs.storage_.get() );

        }

        private:
        void in_place_intersect( container_t& lhs, const container_t& rhs )
        {
        	for( iterator it_lhs = lhs.begin(); it_lhs != lhs.end(); )
        	{
    			bool found = false;
        		for( const_iterator it_rhs = rhs.begin(); it_rhs != rhs.end(); ++ it_rhs )
        		{
        			if( *it_lhs == *it_rhs )
        			{
        				found = true;
        				break;
        			}
        		}
        		if( !found )
        			it_lhs = lhs.erase( it_lhs );
        		else
        			++ it_lhs;
        	}
        }


        public:
        void insert( const T& x )
        {
                if( universal() )
                        return;
                storage_->push_back(x);
        }

        void erase( const T& x )	// erase one element equal to x
        {
                if( universal() )
                        BOOST_ASSERT_MSG( storage_, "erasure from universal set is not supported" );
                iterator it = std::find( storage_->begin(), storage_->end(), x );
                if( it != storage_->end() )
                	storage_->erase(it);
        }

        bool universal() const
        {
                return !storage_;
        }

        bool empty() const
        {
                return !universal() && storage_->empty();
        }

# if defined( ERASER_DEBUG )
        iterator begin()
        {
                return universal()? storage_->end() : storage_->begin();
        }

        const_iterator begin() const
        {
                return universal()? storage_->end() : storage_->begin();
        }

        iterator end()
        {
                return storage_->end();
        }

        const_iterator end() const
        {
                return storage_->end();
        }

        bool operator==( const universal_set& rhs ) const
        {
                if( universal() != rhs.universal() )
                        return false;
                if( universal() )
                        return true;

                const container_t& lhsc = storage_.get();
                const container_t& rhsc = rhs.storage_.get();
                return std::equal( lhsc.begin(), lhsc.end(), rhsc.begin() );
        }
# endif


};

template <class T >
struct shared_var_set : public universal_set<T>
{
        shared_var_set() : universal_set<T>(true){}
        private:
                void erase( const T& );
                void insert( const T& );
};

template <class T >
struct thread_set : public universal_set<T>
{
        thread_set() : universal_set<T>(false){}
        private:
                void intersect_with( const universal_set<T>& );
};

# if defined( ERASER_DEBUG )
template <class T>
std::ostream& operator<<( std::ostream& os, eraser::universal_set<T> uset )
{
        if (uset.universal())
        {
                os << "U";
                return os;
        }
        os << "[ ";
        std::copy( uset.begin(), uset.end(), std::ostream_iterator<T>(os, " ") );
        os << "]";
        return os;
}
# endif

}


#endif
