#pragma once

#include "fsw.h"
#include "timer.h"

using fsw::TimerManager;

#define FSW_EPOLL_CAP 16

namespace fsw {
struct Poll
{
    int epollfd;
    int ncap;
    int event_num;
    struct epoll_event *events;
};

class Event
{
public:
    enum type
    {
        FSW_EVENT_NULL   = 0,
        FSW_EVENT_DEAULT = 1u << 8,
        FSW_EVENT_READ   = 1u << 9,
        FSW_EVENT_WRITE  = 1u << 10,
        FSW_EVENT_RDWR   = FSW_EVENT_READ | FSW_EVENT_WRITE,
        FSW_EVENT_ERROR  = 1u << 11,
    };

    int running;
    struct Poll *poll;
    TimerManager timer_manager;

    std::function<bool()> handle_timer;
    std::function<bool()> handle_io;

    Event();
    ~Event();
    bool wait();

private:
    int num;

    void register_handler();
    bool default_handle_timer();
    bool default_handle_io();
    bool init_Poll();
    bool free_Poll();
};

typedef struct
{
    Event *event;
} Global_t;

extern Global_t FswG;
}
