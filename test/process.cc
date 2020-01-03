#include "fsw/process.h"
#include "fsw/fsw.h"
#include "fsw/coroutine.h"
#include "gtest/gtest.h"

using fsw::Process;
using fsw::Coroutine;

static void child_handler()
{
    std::cout << Coroutine::get_current()->get_cid() << std::endl;
}

TEST(process, construct)
{
    Process process(std::bind(child_handler));
}

TEST(process, construct_enable_coroutine)
{
    bool ret;

    Process process(std::bind(child_handler), true);
    ret = process.start();
    ASSERT_EQ(ret, true);
    ASSERT_EQ(process.pid, getpid());
    process.wait(); // Do not delete this line, or TEST(process, wait) will test fail
}

TEST(process, start)
{
    bool ret;

    Process process(std::bind(child_handler));
    ret = process.start();
    ASSERT_EQ(ret, true);
    ASSERT_EQ(process.pid, getpid());
    process.wait(); // Do not delete this line, or TEST(process, wait) will test fail
}

TEST(process, wait)
{
    bool ret;
    pid_t pid;

    Process process(std::bind(child_handler));
    ret = process.start();
    ASSERT_EQ(ret, true);
    pid = process.wait();
    ASSERT_EQ(process.child_pid, pid);
}