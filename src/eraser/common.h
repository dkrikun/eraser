
#ifndef COMMON_H
#define COMMON_H

#include "eraser/traits.h"
#include "eraser/shared_var.h"
#include "eraser/thread.h"
#include "eraser/lock.h"

namespace eraser
{

typedef thread< jvmti_traits >   	thread_t;
typedef lock				        lock_t;
typedef shared_var< jvmti_traits >  shared_var_t;

}

#endif
