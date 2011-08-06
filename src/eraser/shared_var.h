
#ifndef SHARED_VAR_H
#define SHARED_VAR_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "eraser/shared_var_impl.h"
#include "eraser/thread.h"

namespace eraser
{
        template <class EraserTraits>
        struct shared_var
        {
                typedef typename EraserTraits::field_id_t                       field_id_t;
                typedef typename EraserTraits::thread_id_t                      thread_id_t;
                typedef eraser::thread<EraserTraits>                            thread_t;
                typedef boost::function< void( field_id_t, const thread_t& )>   alarm_func_t;

                field_id_t                                                          field_id_;
                boost::shared_ptr< typename shared_var_impl<EraserTraits>::type >   impl_;

                shared_var( field_id_t );
                shared_var( field_id_t, const alarm_func_t& );
                ~shared_var();

                void read( const thread_t& reader );
                void write( const thread_t& writer );

                field_id_t field_id() const
                {
                        return field_id_;
                }
        };

        template <class EraserTraits>
        std::ostream& operator<<( std::ostream& os, const shared_var<EraserTraits>& sv )
        {
                os << sv.field_id_;
                return os;
        }
}
#endif
