
#include <boost/msm/back/state_machine.hpp>
namespace msm = boost::msm;
 
// forward declare shared_var_impl for pimpl in shared_var.h
namespace eraser
{
        namespace shared_var_fsm
        {
                template <class EraserTraits>
                struct eraser_;
        }

        template <class EraserTraits>
        struct shared_var_impl
        {
                typedef typename msm::back::state_machine< shared_var_fsm::eraser_<EraserTraits> > type;
        };
}

