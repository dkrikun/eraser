
#ifndef THREAD_H
#define THREAD_H

#include <boost/operators.hpp>
#include "eraser/universal_set.h"
#include "eraser/thread_compare.h"

namespace eraser
{
        template <class EraserTraits >
        struct thread : thread_compare<EraserTraits>
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
                }

                void write_lock( lock_id_t lock )
                {
                        locks_held_.insert( lock );
                        write_locks_held_.insert( lock );
                }

                void unlock( lock_id_t lock )
                {
                        locks_held_.erase( lock );
                }

                void write_unlock( lock_id_t lock )
                {
                        locks_held_.erase( lock );
                        write_locks_held_.erase( lock );
                }

                bool operator==(const thread& rhs) const
                {
                        return thread_compare<EraserTraits>::is_same(thread_id_, rhs.thread_id_);
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
