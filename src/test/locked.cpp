

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/bind.hpp>

#include "eraser/shared_var.h"
#include "test/traits.h"

namespace eraser
{

# define TEST_NAME shared_var_locked
# define TEST_(name)\
        TEST_F( TEST_NAME, name )

struct TEST_NAME : public ::testing::Test
{
        typedef shared_var<test_traits>              shared_var_t;
        typedef shared_var_t::thread_t               thread_t;
        typedef shared_var_t::thread_id_t            thread_id_t;
        typedef shared_var_t::field_id_t             field_id_t;
        typedef test_traits::lock_id_t               lock_id_t;

        struct alarm_func
        {
                MOCK_METHOD2( alarm, void( field_id_t, const thread_t& ) );
        };

        alarm_func      alarm_;
        shared_var_t    uut;
        thread_t        thread_1;
        thread_t        thread_2;
        thread_t        thread_3;

        lock_id_t          l1;
        lock_id_t          l2;
        lock_id_t          l3;


        TEST_NAME()
                : uut( field_id_t(0)
                     , boost::bind( &alarm_func::alarm, &alarm_, _1, _2 ) )     // create mock alarm handler
                , thread_1( thread_id_t(1) )
                , thread_2( thread_id_t(2) )
                , thread_3( thread_id_t(3) )
                , l1( lock_id_t(10) )
                , l2( lock_id_t(20) )
                , l3( lock_id_t(30) )
        {}
};

using ::testing::_;

# define EXPECT_NO_ALARM()\
        EXPECT_CALL( alarm_, alarm(_,_) ) \
                .Times(0);

# define EXPECT_ALARM(count)\
        EXPECT_CALL( alarm_, alarm( uut.field_id(), _) )\
                .Times((count));

TEST_( Test_that_concurrent_write_when_locked_ok )
{
        EXPECT_NO_ALARM();

        // init, go to exclusive
        uut.write( thread_3 );

        thread_1.lock( l1 );
        uut.write( thread_1 );
        thread_1.unlock( l1 );

        thread_2.lock( l1 );
        uut.write( thread_2 );
        thread_2.unlock( l1 );
}

TEST_( Test_that_concurrent_write_when_only_one_locked_alarms )
{
        EXPECT_ALARM(1);

        // init, go to exclusive
        uut.write( thread_3 );

        thread_1.lock( l1 );
        uut.write( thread_1 );
        thread_1.unlock( l1 );

        uut.write( thread_2 );
}

TEST_( Test_that_concurrent_write_when_locked_but_another_lock_alarms )
{
        EXPECT_ALARM(1);

        // init, go to exclusive
        uut.write( thread_3 );

        thread_1.lock( l1 );
        uut.write( thread_1 );
        thread_1.unlock( l1 );

        thread_2.lock( l2 );
        uut.write( thread_2 );
        thread_2.unlock( l2 );
}

TEST_( Test_that_concurrent_write_when_locked_with_some_common_lock_ok )
{
        EXPECT_NO_ALARM();

        // init, go to exclusive
        uut.write( thread_3 );

        thread_1.lock( l1 );
        thread_1.lock( l2 );
        uut.write( thread_1 );
        thread_1.unlock( l2 );
        thread_1.unlock( l1 );

        thread_2.lock( l1 );
        thread_2.lock( l3 );
        uut.write( thread_2 );
        thread_2.unlock( l3 );
        thread_2.unlock( l1 );
}

TEST_( Test_that_exclusive_then_concurrent_locked_write_ok )
{
        EXPECT_NO_ALARM();

        // exclusive for thread_1
        uut.write( thread_1 );
        uut.read( thread_1 );
        uut.write( thread_1 );
        uut.read( thread_1 );

        // shared-write, synchronized
        thread_2.lock( l2 );
        uut.write( thread_2 );
        thread_2.unlock( l2 );

        thread_1.lock( l2 );
        uut.write( thread_1 );
        thread_1.unlock( l2 );
}

TEST_( Test_that_exclusive_then_shared_read_locked_then_concurrent_locked_write_ok )
{
        EXPECT_NO_ALARM();

        // exclusive for thread_1
        uut.write( thread_1 );
        uut.read( thread_1 );
        uut.write( thread_1 );
        uut.read( thread_1 );

        // shared-read, synchronized
        thread_3.lock( l2 );
        uut.read( thread_3 );
        thread_3.unlock( l2 );

        thread_2.lock( l2 );
        uut.read( thread_2 );
        thread_2.unlock( l2 );

        // shared-write, synchronized
        thread_2.lock( l2 );
        uut.write( thread_2 );
        thread_2.unlock( l2 );

        thread_1.lock( l2 );
        uut.write( thread_1 );
        thread_1.unlock( l2 );
}

TEST_( Test_that_shared_read_with_different_locks_ok )
{
        EXPECT_NO_ALARM();

        // init, go to exclusive
        uut.write( thread_1 );

        thread_1.lock( l1 );
        uut.read( thread_1 );
        thread_1.unlock( l1 );

        thread_2.lock( l2 );
        uut.read( thread_2 );
        thread_2.unlock( l2 );
}


}
