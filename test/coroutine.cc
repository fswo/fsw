#include <iostream>
#include "fsw/coroutine.h"
#include "gtest/gtest.h"

using namespace fsw;
using namespace std;
using namespace fsw::coroutine;

static int sleep_ii = 0;
static long sleep_ret[2];

static int defer_ii = 0;
static int defer_ret[2];

static void func1()
{
    Coroutine *co = Coroutine::get_current();
    ASSERT_EQ(co->get_cid(), 1);
}

static void func2()
{
    Coroutine *co = Coroutine::get_current();
    ASSERT_EQ(co->get_cid(), 2);
}

TEST(coroutine, get_cid)
{
    Coroutine::create(func1);
    Coroutine::create(func2);
}

static void func3()
{
    Coroutine::sleep(0.002);
    sleep_ret[sleep_ii++] = 1;
}

static void func4()
{
    Coroutine::sleep(0.001);
    sleep_ret[sleep_ii++] = 2;
}

TEST(coroutine, sleep)
{
    run([]()
    {
        Coroutine::create(func3);
        Coroutine::create(func4);
    });
    
    ASSERT_EQ(sleep_ret[0], 2);
    ASSERT_EQ(sleep_ret[1], 1);
}

static void func5()
{
    Coroutine *co = Coroutine::get_current();
    co->defer([](void *arg)
    {
        defer_ret[defer_ii++] = 0;
    });
    co->defer([](void *arg)
    {
        defer_ret[defer_ii++] = 1;
    });
}

TEST(coroutine, defer)
{
    run([]()
    {
        Coroutine::create(func5);
    });
    
    ASSERT_EQ(defer_ret[0], 1);
    ASSERT_EQ(defer_ret[1], 0);
}
