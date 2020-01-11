#pragma once

#include "fsw.h"

namespace fsw
{

class Socket
{
    
public:
    enum type
    {
        FSW_SOCK_TCP          =  1,
        FSW_SOCK_UDP          =  2,
    };

    int fd;

    Socket(int domain, int type, int protocol);
    Socket(int _fd);
    ~Socket();
    bool bind(int type, char *host, int port);
    bool listen(int backlog);
    int accept();
    bool close();
    bool shutdown(int how);
    bool set_option(int level, int optname, const void *optval, socklen_t optlen);
    bool get_option(int level, int optname, void *optval, socklen_t *optlen);
    bool getname(sockaddr *addr, socklen_t *len);
    bool getpeername(sockaddr *addr, socklen_t *len);
    ssize_t recv(void *buf, size_t len, int flag);
    ssize_t send(const void *buf, size_t len, int flag);
    int set_nonblock();
};
}
