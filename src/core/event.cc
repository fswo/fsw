#include "coroutine.h"
#include "log.h"
#include "help.h"
#include "event.h"

using fsw::Coroutine;
using fsw::Event;

fsw::Global_t fsw::FswG;

Event::Event()
{
    init_Poll();
    register_handler();
    running = 1;
}

Event::~Event()
{
    running = 0;
    free_Poll();
}

bool Event::wait()
{
    while (running > 0)
    {
        int64_t timeout;

        timeout = timer_manager.get_next_timeout();
        if (timeout < 0 && poll->event_num == 0)
        {
            running = 0;
            break;
        }
        num = epoll_wait(poll->epollfd, poll->events, poll->ncap, timeout);

        handle_io();
        handle_timer();
    }

    return true;
}

void Event::register_handler()
{
    handle_io = std::bind(&Event::default_handle_io, this);
    handle_timer = std::bind(&Event::default_handle_timer, this);
}

bool Event::default_handle_timer()
{
    timer_manager.run_timers();
    return true;
}

bool Event::default_handle_io()
{
    for (int i = 0; i < num; i++)
    {
        int fd;
        int cid;
        struct epoll_event *p = &(poll->events[i]);
        uint64_t u64 = p->data.u64;

        fsw::help::fromuint64(u64, &fd, &cid);
        fswTrace("coroutine[%d] resume", cid);
        Coroutine::resume(cid);
    }
    return true;
}

bool Event::init_Poll()
{
    poll = new Poll();

    poll->epollfd = epoll_create(256);
    if (poll->epollfd  < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        delete poll;
        poll = nullptr;
        return false;
    }

    poll->ncap = FSW_EPOLL_CAP;
    poll->events = new epoll_event[poll->ncap](); // zero initialized
    poll->event_num = 0;
    return true;
}

bool Event::free_Poll()
{
    if (close(poll->epollfd) < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    delete[] poll->events;
    poll->events = nullptr;
    delete poll;
    poll = nullptr;
    return true;
}
