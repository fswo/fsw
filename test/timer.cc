#include "fsw/fsw.h"
#include "fsw/coroutine.h"
#include "fsw/timer.h"
#include "fsw/event.h"
#include "gtest/gtest.h"
#include <iostream>

using fsw::coroutine::run;
using fsw::Timer;

static int ret[4];
static int ii = 0;

void print_num(void *arg)
{
    ret[ii++] = (int)(uintptr_t)arg;
}

TEST(timer, add_timer)
{
    run([]()
    {
        FswG.event->timer_manager.add_timer(1 * Timer::MILLI_SECOND, print_num, (void *)(uintptr_t)1);
        FswG.event->timer_manager.add_timer(2 * Timer::MILLI_SECOND, print_num, (void *)(uintptr_t)2);
        FswG.event->timer_manager.add_timer(3 * Timer::MILLI_SECOND, print_num, (void *)(uintptr_t)3);
        FswG.event->timer_manager.add_timer(4 * Timer::MILLI_SECOND, print_num, (void *)(uintptr_t)4);
    });

    for (size_t i = 0; i < 4; i++)
    {
        ASSERT_EQ(ret[i], i + 1);
    }
}
