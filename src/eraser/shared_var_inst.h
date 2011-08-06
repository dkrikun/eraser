
#ifndef SHARED_VAR_INST_H
#define SHARED_VAR_INST_H

#include "eraser/traits.h"
#include "test/traits.h"

// explicitly instantiate shared_var template
namespace eraser
{

template class eraser::shared_var<eraser::jvmti_traits>;
template class eraser::shared_var<eraser::test_traits>;

}

#endif
