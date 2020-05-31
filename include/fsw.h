#pragma once

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
#include <sys/sendfile.h>
#include <sys/prctl.h>

#include <queue>
#include <map>
#include <unordered_map>
#include <list>
#include <stack>
#include <string>
#include <iostream>
#include <algorithm>

#include <functional>

#include "config.h"

#ifdef HAVE_EPOLL
#include <sys/epoll.h>
#endif

#define FSW_STRL(str)		(str), (sizeof(str)-1)

enum fswReturn_code
{
    FSW_CONTINUE = 1,
    FSW_WAIT     = 2,
    FSW_CLOSE    = 3,
    FSW_ERROR    = 4,
    FSW_READY    = 5,
};
