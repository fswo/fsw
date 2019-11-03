#include <iostream>
#include "fsw/coroutine.h"
#include "fsw/coroutine_channel.h"
#include "gtest/gtest.h"

using namespace fsw;
using namespace std;
using fsw::coroutine::Channel;

TEST(coroutine_channel, pop_push)
{
    Channel *chan = new Channel();

    Coroutine::create([](void *arg)
    {
        void *data;
        Channel *chan = (Channel *)arg;
        data = chan->pop();
        ASSERT_EQ(*(string *)data, "hello world");
    }, (void *)chan);

    Coroutine::create([](void *arg)
    {
        bool ret;
        string data = "hello world";
        Channel *chan = (Channel *)arg;
        ret = chan->push(&data);
        ASSERT_TRUE(ret);
    }, (void *)chan);

    delete chan;
}
