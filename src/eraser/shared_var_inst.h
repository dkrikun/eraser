
#ifndef SHARED_VAR_INST_H
#define SHARED_VAR_INST_H

#include "test/traits.h"
#include "eraser/traits.h"

// explicitly instantiate shared_var template
namespace eraser
{

#if defined( ERASER_TESTING )
template class eraser::shared_var<eraser::test_traits>;
#else
template class eraser::shared_var<eraser::jvmti_traits>;
#endif


}

#endif
