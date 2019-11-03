#include "fsw/fsw.h"
#include "fsw/timer.h"
#include "gtest/gtest.h"
#include <iostream>

using fsw::Timer;
using fsw::timer_manager;

static int ret[4];
static int ii = 0;

void print_num(void *arg)
{
    ret[ii++] = (int)(uintptr_t)arg;
}

TEST(timer, add_timer)
{
    fsw_event_init();

    timer_manager.add_timer(1 * Timer::MILLI_SECOND, print_num, (void*)(uintptr_t)1);
    timer_manager.add_timer(2 * Timer::MILLI_SECOND, print_num, (void*)(uintptr_t)2);
    timer_manager.add_timer(3 * Timer::MILLI_SECOND, print_num, (void*)(uintptr_t)3);
    timer_manager.add_timer(4 * Timer::MILLI_SECOND, print_num, (void*)(uintptr_t)4);

    fsw_event_wait();
    for (size_t i = 0; i < 4; i++)
    {
        ASSERT_EQ(ret[i], i + 1);
    }
}
