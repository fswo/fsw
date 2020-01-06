#include "socket.h"
#include "log.h"

int fswSocket_create(int domain, int type, int protocol)
{
    int sock;

    sock = socket(domain, type, protocol);
    if (sock < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }

    return sock;
}

bool fswSocket_bind(int sock, int type, char *host, int port)
{
    struct sockaddr_in servaddr;

    if (type == FSW_SOCK_TCP)
    {
        bzero(&servaddr, sizeof(servaddr));
        if (inet_aton(host, &(servaddr.sin_addr)) < 0)
        {
            return false;
        }
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        if (bind(sock, (sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            return false;
        }
    }
    else
    {
        fswWarn("Error has occurred: %s", "the bind type is not supported");
        return false;
    }

    return true;
}

int fswSocket_accept(int sock)
{
    int connfd;
    struct sockaddr_in sa;
    socklen_t len;

    len = sizeof(sa);
    connfd = accept(sock, (struct sockaddr *)&sa, &len);
    return connfd;
}

bool fswSocket_close(int fd)
{
    return close(fd) < 0 ? false : true;
}

bool fswSocket_shutdown(int sock, int how)
{
    return shutdown(sock, how) < 0 ? false : true;
}

bool fswSocket_set_option(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    return setsockopt(fd, level, optname, optval, optlen) < 0 ? false : true;
}

bool fswSocket_get_option(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    return getsockopt(fd, level, optname, optval, optlen) < 0 ? false : true;
}

bool fswSocket_getname(int fd, sockaddr *addr, socklen_t *len)
{
    return getsockname(fd, addr, len) < 0 ? false : true;
}

bool fswSocket_getpeername(int fd, sockaddr *addr, socklen_t *len)
{
    return getpeername(fd, addr, len) < 0 ? false : true;
}

bool fswSocket_listen(int sock, int backlog)
{
    return listen(sock, backlog) < 0 ? false : true;
}

ssize_t fswSocket_recv(int sock, void *buf, size_t len, int flag)
{
    ssize_t ret;

    ret = recv(sock, buf, len, flag);
    return ret;
}

ssize_t fswSocket_send(int sock, const void *buf, size_t len, int flag)
{
    ssize_t ret;

    ret = send(sock, buf, len, flag);
    return ret;
}

int fswSocket_set_nonblock(int sock)
{
    int flags;

    flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0)
    {
        return -1;
    }
    flags = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    return flags;
}