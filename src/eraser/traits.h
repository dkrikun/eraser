
#ifndef TRAITS_H
#define TRAITS_H

#include <jvmti.h>
namespace eraser
{

struct jvmti_traits
{
        typedef jthread thread_id_t;
        typedef jfieldID field_id_t;
        typedef int lock_id_t;
};

}



#endif
