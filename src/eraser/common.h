
#ifndef COMMON_H
#define COMMON_H

#include "eraser/traits.h"
#include "eraser/shared_var.h"
#include "eraser/thread.h"

namespace eraser
{

typedef thread< jvmti_traits >   	thread_t;
typedef jvmti_traits::lock_id_t     lock_t;
typedef shared_var< jvmti_traits >  shared_var_t;

}

#endif
