
#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/bind.hpp>
#include <boost/concept_check.hpp>

#include "eraser/universal_set.h"

namespace eraser
{

# define TEST_NAME universal_set_test
# define TEST_(name)\
        TEST_F( TEST_NAME, name )

// will test our universal_set on ints for now
// only need to be able to create new or same values for the test

struct TEST_NAME : public ::testing::Test
{
        typedef int                                     elem_t;
        typedef eraser::thread_set< elem_t >            thread_set_t;
        typedef eraser::shared_var_set< elem_t >        shared_var_set_t;

        elem_t make_new()
        {
                return current_++;
        }
        elem_t make_same( const elem_t& e )
        {
                return e;
        }

        elem_t current_;
        universal_set_test()
                : current_(0)
        {}
};


TEST_( test_that_thread_set_is_not_universal )
{
        thread_set_t uut;
        EXPECT_FALSE( uut.universal() );
}

TEST_( test_that_thread_set_is_empty_initially )
{
        thread_set_t uut;
        EXPECT_TRUE( uut.empty() );
}

TEST_( test_that_shared_var_set_is_universal_initially )
{
        shared_var_set_t uut;
        EXPECT_TRUE( uut.universal() );
}

TEST_( test_that_shared_var_set_is_not_empty_initially )
{
        shared_var_set_t uut;
        EXPECT_FALSE( uut.empty() );
}

TEST_( test_that_thread_set_can_erase_one_correctly )
{
        thread_set_t uut;
        elem_t e1 = make_new();

        uut.insert( e1 );

        EXPECT_FALSE( uut.empty() );
        EXPECT_EQ( uut.erase( e1 ), 1 );
        EXPECT_TRUE( uut.empty() );
}

TEST_( test_that_thread_set_no_duplicates )
{
        thread_set_t uut;
        elem_t e1 = make_new();
        elem_t e2 = make_new();
        elem_t e3 = make_same( e1 );        

        uut.insert( e1 );
        uut.insert( e2 );
        uut.insert( e3 );

        EXPECT_EQ( uut.erase( e1 ), 1 );
        EXPECT_EQ( uut.erase( e2 ), 1 );
        EXPECT_EQ( uut.erase( e3 ), 0 );
        EXPECT_TRUE( uut.empty() );
}

TEST_( test_that_shared_var_set_intersected_with_phi_yields_phi )
{
        shared_var_set_t uut;
        thread_set_t phi;

        uut.intersect_with( phi );
        EXPECT_TRUE( uut.empty() );
}

TEST_( test_shared_var_set_intersections_various_1 )
{
        shared_var_set_t uut;

        elem_t e1 = make_new();
        elem_t e2 = make_new();

        // ts1 = { e1 }
        thread_set_t ts1;
        ts1.insert( e1 );

        // ts2 = { e2 }
        thread_set_t ts2;
        ts2.insert( e2 );

        uut.intersect_with( ts1 );
        EXPECT_FALSE( uut.empty() );
        uut.intersect_with( ts2 );
        EXPECT_TRUE( uut.empty() );
}

TEST_( test_shared_var_set_intersections_various_2 )
{
        shared_var_set_t uut;

        elem_t e1 = make_new();
        elem_t e2 = make_new();
        elem_t e3 = make_new();
        elem_t e4 = make_new();
        elem_t e5 = make_new();
        elem_t e6 = make_new();

        // ts1 = { e1, e2 }
        thread_set_t ts1;
        ts1.insert( e1 );
        ts1.insert( e2 );

        // ts2 = { e1, e2, e4, e5, e6 }
        thread_set_t ts2;
        ts2.insert( e1 );
        ts2.insert( e2 );
        ts2.insert( e4 );
        ts2.insert( e5 );
        ts2.insert( e6 );

        // ts3 = { e3, e2, e6 }
        thread_set_t ts3;
        ts3.insert( e3 );
        ts3.insert( e2 );
        ts3.insert( e6 );

        // ts4 = { e5 }
        thread_set_t ts4;
        ts4.insert( e5 );

        uut.intersect_with( ts2 );
        EXPECT_FALSE( uut.universal() );
        EXPECT_FALSE( uut.empty() );
        uut.intersect_with( ts1 );
        EXPECT_FALSE( uut.empty() );
        uut.intersect_with( ts3 );
        EXPECT_FALSE( uut.empty() );
        uut.intersect_with( ts4 );
        EXPECT_TRUE( uut.empty() );
}
        
TEST_( test_shared_var_set_intersections_various_3 )
{
        shared_var_set_t uut;

        elem_t e1 = make_new();
        elem_t e2 = make_new();
        elem_t e3 = make_new();
        elem_t e4 = make_new();

        // ts1 = { e1, e3 }
        thread_set_t ts1;
        ts1.insert( e1 );
        ts1.insert( e3 );

        // ts2 = { e2, e3 }
        thread_set_t ts2;
        ts2.insert( e2 );
        ts2.insert( e3 );
          
        // ts3 = { e4 }
        thread_set_t ts3;
        ts3.insert( e4 );

        uut.intersect_with( ts1 );
        EXPECT_FALSE( uut.empty() );
        uut.intersect_with( ts2 );
        EXPECT_FALSE( uut.empty() );
        uut.intersect_with( ts3 );
        EXPECT_TRUE( uut.empty() );
}


}
