#include "coroutine.h"
#include "coroutine_channel.h"
#include "timer.h"

using fsw::Coroutine;
using fsw::coroutine::Channel;
using fsw::Timer;
using fsw::TimerManager;

Channel::Channel(size_t _capacity):
    capacity(_capacity)
{
}

Channel::~Channel()
{
}

static void sleep_timeout(void *param)
{
    Coroutine::resume((Coroutine *) param);
}

void* Channel::pop(double timeout)
{
    Coroutine *co = Coroutine::get_current();
    void *data;

    if (data_queue.empty())
    {
        if (timeout > 0)
        {
            FE(timer_manager).add_timer(timeout * Timer::SECOND, sleep_timeout, (void*)co);
        }
        consumer_queue.push(co);
        Coroutine::yield();;
    }

    if (data_queue.empty())
    {
        return nullptr;
    }

    data = data_queue.front();
    data_queue.pop();

    /**
     * notice producer
     */
    if (!producer_queue.empty())
    {
        co = producer_queue.front();
        producer_queue.pop();
        Coroutine::resume(co);
    }

    return data;
}

bool Channel::push(void *data, double timeout)
{
    Coroutine *co = Coroutine::get_current();
    if (data_queue.size() == capacity)
    {
        if (timeout > 0)
        {
            FE(timer_manager).add_timer(timeout * Timer::SECOND, sleep_timeout, (void*)co);
        }
        producer_queue.push(co);
        Coroutine::yield();;
    }

    /**
     * channel full
     */
    if (data_queue.size() == capacity)
    {
        return false;
    }

    data_queue.push(data);

    /**
     * notice consumer
     */
    if (!consumer_queue.empty())
    {
        co = consumer_queue.front();
        consumer_queue.pop();
        Coroutine::resume(co);
    }

    return true;
}