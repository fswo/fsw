#ifndef EVENT_H_
#define EVENT_H_

#include "fsw.h"

#define FSW_EPOLL_CAP 16;

namespace fsw { namespace event {

typedef struct
{
    int epollfd;
    int ncap;
    int event_num;
    struct epoll_event *events;
} fswPoll_t;

typedef struct
{
    int running;
    fswPoll_t *poll;
} fswGlobal_t;

extern fswGlobal_t FswG;

enum fswEvent_type
{
    FSW_EVENT_NULL   = 0,
    FSW_EVENT_DEAULT = 1u << 8,
    FSW_EVENT_READ   = 1u << 9,
    FSW_EVENT_WRITE  = 1u << 10,
    FSW_EVENT_RDWR   = FSW_EVENT_READ | FSW_EVENT_WRITE,
    FSW_EVENT_ERROR  = 1u << 11,
};

int init_fswPoll();
inline int free_fswPoll();

int fsw_event_init();
int fsw_event_wait();
int fsw_event_free();

}
}

#endif /* EVENT_H_ */