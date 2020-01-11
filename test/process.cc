#include "gtest/gtest.h"
#include "fsw/fsw.h"
#include "fsw/process.h"
#include "fsw/buffer.h"
#include "fsw/coroutine.h"

using fsw::Process;
using fsw::Coroutine;
using fsw::Buffer;

static void child_handler(Process *process)
{
}

static void name_child_handler(Process *process)
{
    process->name("worker");
}

TEST(process, construct)
{
    Process process(child_handler);
}

TEST(process, construct_enable_coroutine)
{
    bool ret;

    Process process(child_handler, true);
    ret = process.start();
    ASSERT_EQ(ret, true);
    ASSERT_EQ(process.pid, getpid());
    process.wait(); // Do not delete this line, or TEST(process, wait) will test fail
}

TEST(process, start)
{
    bool ret;

    Process process(child_handler);
    ret = process.start();
    ASSERT_EQ(ret, true);
    ASSERT_EQ(process.pid, getpid());
    process.wait(); // Do not delete this line, or TEST(process, wait) will test fail
}

TEST(process, wait)
{
    bool ret;
    pid_t pid;

    Process process(child_handler);
    ret = process.start();
    ASSERT_EQ(ret, true);
    pid = process.wait();
    ASSERT_EQ(process.child_pid, pid);
}

TEST(process, name)
{
    bool ret;
    pid_t pid;

    Process process(name_child_handler);
    ret = process.start();
    process.name("master");
    process.wait();
}

static void read_child_handler(Process *process)
{
    int ret;
    char *buff = process->socket->get_read_buf()->c_buffer();

    ret = process->read(buff, 1024);
    buff[ret] = 0;
    ASSERT_STREQ(buff, "hello");
}

TEST(process, write_read)
{
    ssize_t ret;
    pid_t pid;

    Process process(read_child_handler);
    process.start();
    Buffer *buff = process.socket->get_write_buf();
    buff->append("hello");
    ret = process.write(buff->c_buffer(), buff->length());
    ASSERT_EQ(ret, buff->length());
    pid = process.wait();
}