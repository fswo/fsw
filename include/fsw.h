#ifndef FSW_H_
#define FSW_H_

// include standard library
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/stat.h>

#include <queue>
#include <map>
#include <list>
#include <stack>
#include <string>
#include <iostream>
#include <algorithm>

#include "config.h"

#ifdef HAVE_EPOLL
#include <sys/epoll.h>
#endif

#define FSW_EPOLL_CAP 16;

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

static inline uint64_t touint64(int fd, int id)
{
    uint64_t ret = 0;
    ret |= ((uint64_t)fd) << 32;
    ret |= ((uint64_t)id);

    return ret;
}

static inline void fromuint64(uint64_t v, int *fd, int *id)
{
    *fd = (int)(v >> 32);
    *id = (int)(v & 0xffffffff);
}

#endif /* FSW_H_ */