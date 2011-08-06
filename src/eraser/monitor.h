
#ifndef MONITOR_H
#define MONITOR_H

#include <jvmti.h>
#include "sun/agent_util.h"

namespace eraser
{

inline void monitor_enter( jvmtiEnv* jvmti, jrawMonitorID monitor )
{
        jvmtiError err;
        err = jvmti->RawMonitorEnter(monitor);
        check_jvmti_error(jvmti, err, "raw monitor enter");
}

inline void monitor_exit( jvmtiEnv* jvmti, jrawMonitorID monitor )
{
        jvmtiError err;
        err = jvmti->RawMonitorExit(monitor);
        check_jvmti_error(jvmti, err, "raw monitor exit");
}

struct scoped_lock
{
        jvmtiEnv*       jvmti_;
        jrawMonitorID   monitor_;

        scoped_lock( jvmtiEnv* jvmti, jrawMonitorID monitor )
                : jvmti_(jvmti)
                , monitor_(monitor)
        {
                eraser::monitor_enter(jvmti_, monitor_);
        }

        ~scoped_lock()
        {
                eraser::monitor_exit(jvmti_, monitor_);
        }
};

}

#endif
