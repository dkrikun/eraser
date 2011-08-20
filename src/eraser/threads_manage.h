#ifndef THREADS_MANAGE_H
#define THREADS_MANAGE_H


#include <jvmti.h>
#include "eraser/common.h"

namespace eraser
{

thread_t* get_thread( jthread thread_id, std::string debug );
thread_t* get_thread( jthread thread_id );

}

#endif
