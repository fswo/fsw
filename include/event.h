#ifndef EVENT_H_
#define EVENT_H_

#include "fsw.h"

#define FSW_EPOLL_CAP 16;

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

    Event();
    ~Event();
    int wait();

private:
    int init_Poll();
    int free_Poll();
};

typedef struct
{
    Event *event;
} Global_t;

extern Global_t FswG;
}

#endif /* EVENT_H_ */