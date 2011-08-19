
#ifndef THREAD_H
#define THREAD_H

#include <boost/operators.hpp>
#include "eraser/universal_set.h"
#include "eraser/thread_compare.h"
#include "eraser/logger.h"

namespace eraser
{
        template <class EraserTraits >
        struct thread : thread_compare<typename EraserTraits::thread_id_t>
        			  , boost::operators< thread<EraserTraits> >
        {
                typedef typename EraserTraits::thread_id_t        thread_id_t;
                typedef typename EraserTraits::lock_id_t          lock_id_t;

                thread( thread_id_t thread_id )
                        : thread_id_(thread_id)
                {}

                // using default copy-ctor & assignment

                thread_id_t                thread_id_;
                thread_set< lock_id_t >    locks_held_;
                thread_set< lock_id_t >    write_locks_held_;

                void lock( lock_id_t lock )
                {
                        locks_held_.insert( lock );
                        LOG_INFO( "locks_held: " << locks_held_ );
                }


                void unlock( lock_id_t lock )
                {
                        locks_held_.erase( lock );
                        LOG_INFO( "locks_held: " << locks_held_ );
                }

                bool operator==(const thread& rhs) const
                {
                        return thread_compare<thread_id_t>::is_equal(thread_id_, rhs.thread_id_);
                }
        };

        template <class EraserTraits>
        std::ostream& operator<<( std::ostream& os, const thread<EraserTraits> t )
        {
        	os << t.thread_id_;
        	return os;
        }
}

#endif
