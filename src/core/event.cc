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
        int n;
        int64_t timeout;
        epoll_event *events;

        timeout = timer_manager.get_next_timeout();
        events = poll->events;
        if (timeout < 0 && poll->event_num == 0)
        {
            running = 0;
            break;
        }
        n = epoll_wait(poll->epollfd, events, poll->ncap, timeout);
        
        for (int i = 0; i < n; i++)
        {
            int fd;
            int cid;
            struct epoll_event *p = &events[i];
            uint64_t u64 = p->data.u64;

            fsw::help::fromuint64(u64, &fd, &cid);
            fswTrace("coroutine[%d] resume", cid);
            Coroutine::resume(cid);
        }

        timer_manager.run_timers();
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
