
#include <iostream>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>
#include <boost/bind.hpp>
#include <boost/optional/optional.hpp>

#include "eraser/shared_var.h"
#include "eraser/logger.h"
#include "eraser/lock_policy.h"


namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;
using namespace msm::front::euml;

namespace eraser
{
namespace shared_var_fsm
{
    #define xxstr(x) #x
    #define xstr(x) xxstr(x)

    // events
    template <class EraserTraits>
    struct write
    {
            typedef typename eraser::thread<EraserTraits>   thread_t;
            write( const thread_t& writer )
                    : accessing_thread_(writer)
            {}
            const thread_t& accessing_thread_;          // must be the same field name for
    };                                                  // static polymorphism to work

    template <class EraserTraits>
    struct read
    {
            typedef typename eraser::thread<EraserTraits>   thread_t;
            read( const thread_t& reader )
                    : accessing_thread_(reader)
            {}
            const thread_t& accessing_thread_;
    };

    // front-end: define the FSM structure 
    template <class EraserTraits>
    struct eraser_ : public msm::front::state_machine_def< eraser_<EraserTraits> >
    			   , public eraser::lock_policy<EraserTraits>
    {
        typedef typename eraser::thread<EraserTraits>   thread_t;
        typedef typename EraserTraits::lock_id_t        lock_t;
        typedef eraser::lock_policy<EraserTraits>		lock_policy_t;

        typedef boost::function< void(const thread_t&) > alarm_func_bound_t;    // field_id is already bound
        eraser_( const alarm_func_bound_t& alarm )
                : alarm_( alarm )
        {}

        // no alarm will be issued if this ctor is used
        eraser_()
                : alarm_(0)
        {}

        shared_var_set< lock_t >   cv_;
        alarm_func_bound_t         alarm_;

        void update_cv( const thread_set< lock_t >& locks_held )
        {
                cv_.intersect_with( locks_held );
        }

        bool cv_empty() const
        {
                return cv_.empty();
        }
        

        // states
        struct virgin : public msm::front::state<> {};
        struct exclusive : public msm::front::state<>
        {
        		// using default ctor & assignment
                boost::optional<thread_t> last_accessing_thread_;

                template <class Event,class FSM>
                void on_entry(Event const& e, FSM& fsm)
                {
                		fsm.global_lock();
                		logger::instance()->level(0) << "last acc. thread=";
                		if( last_accessing_thread_ )
                			logger::instance()->level(0) << *last_accessing_thread_ << std::endl;
                		else
                			logger::instance()->level(0)  << "null" << std::endl;
                		logger::instance()->level(0)
                				<< "curr. acc. thread=" << e.accessing_thread_ << std::endl;
                		fsm.global_unlock();
                        last_accessing_thread_ = e.accessing_thread_;
                }
        };

        struct shared_read : public msm::front::state<>
        {
                template <class Event,class FSM>
                void on_entry(Event const& e, FSM& fsm)
                {
                        // update cv
                        fsm.update_cv( e.accessing_thread_.locks_held_ );
                }
        };

        struct shared_modified : public msm::front::state<>
        {
                template <class Event,class FSM>
                void on_entry(Event const& e, FSM& fsm)
                {
                		fsm.global_lock();
                		eraser::logger::instance()->level(1) << "CV: " << fsm.cv_ << std::endl;
                		eraser::logger::instance()->level(1) << "locks_held: " << e.accessing_thread_.locks_held_ << std::endl;
                		fsm.global_unlock();
                        // update cv & check data races
                        fsm.update_cv( e.accessing_thread_.locks_held_ );
                        if( fsm.cv_empty() && fsm.alarm_ )
                                fsm.alarm_( e.accessing_thread_ );
                }
        };

        typedef virgin initial_state;

        // transition actions
        struct dbg_trans 
        {
                template <class EVT,class FSM,class SourceState,class TargetState>
                void operator()(EVT const& e, FSM& fsm, SourceState& s ,TargetState& t )
                {
                	fsm.global_lock();
                	eraser::logger::instance()->level(2) << "on " << typeid(EVT).name() << ": "
                        << typeid(SourceState).name() << " --> " << typeid(TargetState).name() << std::endl;
                	fsm.global_unlock();
                }
        };

#       if defined( ERASER_DEBUG )
#             define NONE_DBG dbg_trans
#       else
#             define NONE_DBG none
#       endif

        // guard conditions
        struct new_thread 
        {
                template <class EVT,class FSM,class SourceState,class TargetState>
                bool operator()(EVT const& evt ,FSM& fsm,SourceState& from,TargetState& )
                {
                    BOOST_ASSERT_MSG( from.last_accessing_thread_
                    		, "last_accessing_thread_ is supposed to be init. at this point!" );
                    bool res = evt.accessing_thread_ != from.last_accessing_thread_.get();
#                   if 0 //defined( ERASER_DEBUG )
                    std::cout << "accessing thread= " << evt.accessing_thread_ << std::endl;
                    std::cout << "last acc. thread= " << from.last_accessing_thread_.get() << std::endl;
                    std::cout << "guard condition= " << std::boolalpha << res << std::endl;
#                   endif
                    return res;
                }
        };
 
        typedef eraser::shared_var_fsm::read< EraserTraits >     read;
        typedef eraser::shared_var_fsm::write< EraserTraits >    write;

        // transition table
        struct transition_table : mpl::vector<
            //    start             event                next               action            guard
            //  +------------------------------------------------------------------------------------------------+
            Row < virgin          , write              , exclusive        , NONE_DBG        , none              >,
            Row < virgin          , read               , exclusive        , NONE_DBG        , none              >,
            //  +------------------------------------------------------------------------------------------------+
            Row < exclusive       , read               , exclusive        , NONE_DBG        , Not_<new_thread>  >,
            Row < exclusive       , write              , exclusive        , NONE_DBG        , Not_<new_thread>  >,
            Row < exclusive       , read               , shared_read      , NONE_DBG        , new_thread        >,
            Row < exclusive       , write              , shared_modified  , NONE_DBG        , new_thread        >,
            //  +------------------------------------------------------------------------------------------------+
            Row < shared_read     , read               , shared_read      , NONE_DBG        , none              >,
            Row < shared_read     , write              , shared_modified  , NONE_DBG        , none              >,
            //  +------------------------------------------------------------------------------------------------+
            Row < shared_modified , read               , shared_modified  , NONE_DBG        , none              >,
            Row < shared_modified , write              , shared_modified  , NONE_DBG        , none              >
            //  +------------------------------------------------------------------------------------------------+
        > {};

        // replaces the default no-transition response
        template <class FSM,class Event>
        void no_transition(Event const& e, FSM& fsm,int state)
        {
        	fsm.global_lock();
        	eraser::logger::instance()->level(2) << "NO TRANS on "
        			<< typeid(Event).name() << " from " << state << std::endl;
            BOOST_ASSERT_MSG(false, "No transition in fsm");
            fsm.global_unlock();
        }
    };

    } // namespace shared_var_fsm

   
    template <class EraserTraits>
    shared_var<EraserTraits>::shared_var( field_id_t            field_id
                                       , const alarm_func_t&    alarm )

                                       : field_id_( field_id )
                                       , impl_( new typename shared_var_impl<EraserTraits>::type(
                                                               boost::bind( alarm, field_id, _1 )
                                                       ))
    {
            impl_->start();
    }



    // no alarm will be issued if this ctor is used
    template <class EraserTraits>
    shared_var<EraserTraits>::shared_var( field_id_t field_id )
                                        : field_id_( field_id )
                                        , impl_( new typename shared_var_impl<EraserTraits>::type() )
    {
            impl_->start();
    }


    template <class EraserTraits>
    shared_var<EraserTraits>::~shared_var()
    {
    }

    template <class EraserTraits>
    void shared_var<EraserTraits>::read( const thread_t& reader )
    {
            impl_->process_event( shared_var_fsm::read<EraserTraits>(reader) );
    }

    template <class EraserTraits>
    void shared_var<EraserTraits>::write( const thread_t& writer )
    {
            impl_->process_event( shared_var_fsm::write<EraserTraits>(writer) );
    }

}
// explicitly instantiate the shared_var template
#include "eraser/shared_var_inst.h"
