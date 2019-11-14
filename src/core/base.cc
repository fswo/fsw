#include "fsw.h"
#include "coroutine.h"
#include "log.h"
#include "timer.h"

using fsw::Coroutine;
using fsw::Timer;
using fsw::TimerManager;
using fsw::timer_manager;

fswGlobal_t FswG;

int init_fswPoll()
{
    try
    {
        FswG.poll = new fswPoll_t();
    }
    catch(const std::bad_alloc& e)
    {
        fswError("%s", e.what());
    }

    FswG.poll->epollfd = epoll_create(256);
    if (FswG.poll->epollfd  < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        delete FswG.poll;
        FswG.poll = nullptr;
        return -1;
    }

    FswG.poll->ncap = FSW_EPOLL_CAP;
    try
    {
        FswG.poll->events = new epoll_event[FswG.poll->ncap](); // zero initialized
    }
    catch(const std::bad_alloc& e)
    {
        fswError("%s", e.what());
    }
    
    FswG.poll->event_num = 0;

    return 0;
}

inline int free_fswPoll()
{
    if (close(FswG.poll->epollfd) < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    delete[] FswG.poll->events;
    FswG.poll->events = nullptr;
    delete FswG.poll;
    FswG.poll = nullptr;
    return 0;
}

int fsw_event_init()
{
    if (!FswG.poll)
    {
        init_fswPoll();
    }

    FswG.running = 1;

    return 0;
}

int fsw_event_wait()
{
    fsw_event_init();

    while (FswG.running > 0)
    {
        int n;
        int64_t timeout;
        epoll_event *events;

        timeout = timer_manager.get_next_timeout();
        events = FswG.poll->events;
        if (timeout < 0 && FswG.poll->event_num == 0)
        {
            FswG.running = 0;
            break;
        }
        n = epoll_wait(FswG.poll->epollfd, events, FswG.poll->ncap, timeout);
        
        for (int i = 0; i < n; i++)
        {
            int fd;
            int cid;
            struct epoll_event *p = &events[i];
            uint64_t u64 = p->data.u64;

            fromuint64(u64, &fd, &cid);
            fswTrace("coroutine[%d] resume", cid);
            Coroutine::resume(cid);
        }

        timer_manager.run_timers();
    }

    fsw_event_free();

    return 0;
}

int fsw_event_free()
{
    FswG.running = 0;
    free_fswPoll();
    return 0;
}