
#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/bind.hpp>

#include "eraser/shared_var.h"
#include "test/traits.h"

namespace eraser
{

# define TEST_NAME shared_var_test
# define TEST_(name)\
        TEST_F( TEST_NAME, name )

struct TEST_NAME : public ::testing::Test
{
        typedef shared_var<test_traits>              shared_var_t;
        typedef shared_var_t::thread_t               thread_t;
        typedef shared_var_t::thread_id_t            thread_id_t;
        typedef shared_var_t::field_id_t             field_id_t;

        struct alarm_func
        {
                MOCK_METHOD2( alarm, void( field_id_t, const thread_t& ) );
        };

        alarm_func      alarm_;
        shared_var_t    uut;
        thread_t        thread_1;
        thread_t        thread_2;
        thread_t        thread_3;


        TEST_NAME()
                : uut( field_id_t(0)
                     , boost::bind( &alarm_func::alarm, &alarm_, _1, _2 ) )     // create mock alarm handler
                , thread_1( thread_id_t(1) )
                , thread_2( thread_id_t(2) )
                , thread_3( thread_id_t(3) )
        {}
};

using ::testing::_;

# define EXPECT_NO_ALARM()\
        EXPECT_CALL( alarm_, alarm(_,_) ) \
                .Times(0);

# define EXPECT_ALARM(count)\
        EXPECT_CALL( alarm_, alarm( uut.field_id(), _) )\
                .Times((count));


TEST_( test_that_virgin_write_ok )
{
        EXPECT_NO_ALARM(); 

        uut.write( thread_1 );
}
 
TEST_( test_that_exclusive_write_and_read_same_thread_ok )
{
        EXPECT_NO_ALARM(); 
        
        uut.write( thread_1 );
        uut.write( thread_1 );
        uut.read( thread_1 );
        uut.read( thread_1 );
        uut.write( thread_1 );
        uut.read( thread_1 );
        uut.write( thread_1 );
}

TEST_( test_that_shared_read_ok )
{
        EXPECT_NO_ALARM(); 
        
        uut.write( thread_1 );

        uut.read( thread_1 );
        uut.read( thread_2 );
        uut.read( thread_3 );
        uut.read( thread_2 );
        uut.read( thread_1 );
}

TEST_( test_that_concurrent_write_when_unlocked_alarms_once )
{
        EXPECT_ALARM(1);

        uut.write( thread_1 );
        uut.write( thread_2 );
}

TEST_( test_that_concurrent_write_when_unlocked_alarms_count )
{
        EXPECT_CALL( alarm_, alarm( uut.field_id(), thread_1 ) )
                .Times(2);
        EXPECT_CALL( alarm_, alarm( uut.field_id(), thread_2 ) )
                .Times(1);
        EXPECT_CALL( alarm_, alarm( uut.field_id(), thread_3 ) )
                .Times(3);

        uut.write( thread_1 );
        uut.write( thread_2 );
        uut.write( thread_3 );
        uut.write( thread_3 );
        uut.write( thread_1 );
        uut.write( thread_3 );
        uut.write( thread_1 );
}


TEST_( test_that_concurrent_write_after_shared_read_by_new_thread_when_unlocked_alarms_once )
{
        EXPECT_ALARM(1);

        uut.write( thread_1 );
        uut.read( thread_2 );   // transition to shared-read
        uut.write( thread_3 );  // transition to shared-write
}

TEST_( test_that_concurrent_write_after_shared_read_by_reading_thread_when_unlocked_alarms_once )
{
        EXPECT_ALARM(1);

        uut.write( thread_1 );
        uut.read( thread_2 );   // transition to shared-read
        uut.write( thread_2 );  // transition to shared-write (by reading thread)
}

TEST_( test_that_concurrent_write_after_shared_read_by_old_thread_when_unlocked_alarms_once )
{
        EXPECT_ALARM(1);

        uut.write( thread_1 );
        uut.read( thread_2 );   // transition to shared-read
        uut.write( thread_1 );  // transition to shared-write (by first thread)
}



}
