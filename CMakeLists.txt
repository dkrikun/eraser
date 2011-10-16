# Written for Ubuntu 10.10

cmake_minimum_required(VERSION 2.6)
project(eraser_academic)

# You would probably need to change this path
set(JDK_ROOT "/usr/lib/jvm/default-java/include")
include_directories(${JDK_ROOT})
include_directories(${JDK_ROOT}/linux)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.46.0 REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

add_definitions(-DERASER_DEBUG)

include_directories(src)
include_directories(src/sun)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
SET(CMAKE_LIBRARY_OUTPUT_PATH bin)

set(sources
        src/eraser/agent.cpp
        src/eraser/agent.h
        src/eraser/assert_handler.h
        src/eraser/common.h
        src/eraser/crw_hooks.cpp
        src/eraser/crw_hooks.h
        src/eraser/instrumentation.cpp
        src/eraser/instrumentation.h
        src/eraser/lock.cpp
        src/eraser/lock.h
        src/eraser/lock_policy.h
        src/eraser/logger.h
        src/eraser/main.cpp
        src/eraser/monitor.h
        src/eraser/Proxy.java
        src/eraser/shared_var.cpp
        src/eraser/shared_var.h
        src/eraser/shared_var_impl.h
        src/eraser/shared_var_inst.h
        src/eraser/shared_vars_manage.h
        src/eraser/static_vars.h
        src/eraser/tag.h
        src/eraser/tags
        src/eraser/thread_compare.h
        src/eraser/thread.h
        src/eraser/threads_manage.cpp
        src/eraser/threads_manage.h
        src/eraser/traits.h
        src/eraser/universal_set.h
        src/eraser/vm_hooks.cpp
        src/eraser/vm_hooks.h

        src/sun/agent_util.h
        src/sun/agent_util.c
        src/sun/java_crw_demo.h
        src/sun/java_crw_demo.c
        )

add_library(eraser SHARED ${sources})
if(CMAKE_COMPILER_IS_GNUCXX)
            set(CMAKE_CXX_FLAGS "-Wno-deprecated -Wno-write-strings -Wno-sign-compare")
endif()
target_link_libraries(eraser ${Boost_LIBRARIES})                                                                                                                                                                                                                            
enable_testing(true)
find_package(Threads REQUIRED)
set(GMOCK_ROOT "/home/daniel/work/tools/gmock-1.6.0")
include_directories(${GMOCK_ROOT}/include)
include_directories(${GMOCK_ROOT}/gtest/include)

add_executable(universal_set 
        src/test/universal_set.cpp)
target_link_libraries(universal_set ${GMOCK_ROOT}/libgmock_main.a ${GMOCK_ROOT}/libgmock.a ${GMOCK_ROOT}/gtest/libgtest.a ${CMAKE_THREAD_LIBS_INIT})
add_test(universal_set_test universal_set)

add_executable(shared_var 
        src/test/shared_var.cpp 
        src/eraser/shared_var.cpp 
        src/eraser/universal_set.h)
target_link_libraries(shared_var ${GMOCK_ROOT}/libgmock_main.a ${GMOCK_ROOT}/libgmock.a ${GMOCK_ROOT}/gtest/libgtest.a ${CMAKE_THREAD_LIBS_INIT})
add_definitions(-DERASER_TESTING)
add_test(shared_var_test shared_var)

add_executable(locked 
        src/test/locked.cpp 
        src/eraser/shared_var.cpp 
        src/eraser/universal_set.h)
target_link_libraries(locked ${GMOCK_ROOT}/libgmock_main.a ${GMOCK_ROOT}/libgmock.a ${GMOCK_ROOT}/gtest/libgtest.a ${CMAKE_THREAD_LIBS_INIT})
add_test(locked_test locked)
