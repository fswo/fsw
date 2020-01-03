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

int fswSocket_bind(int sock, int type, char *host, int port)
{
    struct sockaddr_in servaddr;

    if (type == FSW_SOCK_TCP)
    {
        bzero(&servaddr, sizeof(servaddr));
        if (inet_aton(host, &(servaddr.sin_addr)) < 0)
        {
            return -1;
        }
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        if (bind(sock, (sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            return -1;
        }
    }
    else
    {
        fswWarn("Error has occurred: %s", "the bind type is not supported");
        return -1;
    }

    return 0;
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

int fswSocket_close(int fd)
{
    int ret;

    ret = close(fd);
    return ret;
}

int fswSocket_shutdown(int sock, int how)
{
    int ret;

    ret = shutdown(sock, how);
    return ret;
}

int fswSocket_set_option(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    int ret;

    ret = setsockopt(fd, level, optname, optval, optlen);
    return ret;
}

int fswSocket_get_option(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    int ret;

    ret = getsockopt(fd, level, optname, optval, optlen);
    return ret;
}

int fswSocket_getname(int fd, sockaddr *addr, socklen_t *len)
{
    int ret;

    ret = getsockname(fd, addr, len);
    return ret;
}

int fswSocket_getpeername(int fd, sockaddr *addr, socklen_t *len)
{
    int ret;

    ret = getpeername(fd, addr, len);
    return ret;
}

int fswSocket_listen(int sock, int backlog)
{
    int ret;

    ret = listen(sock, backlog);
    return ret;
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