#include <iostream>
#include "fsw/coroutine.h"
#include "fsw/coroutine_channel.h"
#include "gtest/gtest.h"

using namespace fsw;
using namespace std;
using fsw::coroutine::Channel;

static void pop(Channel *chan)
{
    void *data;
    data = chan->pop();
    ASSERT_EQ(*(string *)data, "hello world");
}

static void push(Channel *chan)
{
    bool ret;
    string data = "hello world";
    ret = chan->push(&data);
    ASSERT_TRUE(ret);
}

TEST(coroutine_channel, pop_push)
{
    Channel *chan = new Channel();

    Coroutine::create(std::bind(pop, chan));
    Coroutine::create(std::bind(push, chan));

    delete chan;
}
