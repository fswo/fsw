#include "socket.h"
#include "log.h"

int fswSocket_create(int domain, int type, int protocol)
{
    int on = 1;
    int sock;

    sock = socket(domain, type, protocol);
    if (sock < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    return sock;
}

int fswSocket_bind(int sock, int type, char *host, int port)
{
    int ret;
    struct sockaddr_in servaddr;

    if (type == FSW_SOCK_TCP)
    {
        bzero(&servaddr, sizeof(servaddr));
        if (inet_aton(host, &(servaddr.sin_addr)) < 0)
        {
            fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        }
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        ret = bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (ret < 0)
        {
            fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
            return -1; 
        }
    }
    else
    {
        return -1;
    }

    return ret;
}

int fswSocket_accept(int sock)
{
    int connfd;
    struct sockaddr_in sa;
    socklen_t len;

    len = sizeof(sa);
    connfd = accept(sock, (struct sockaddr *)&sa, &len);
    if (connfd < 0 && errno != EAGAIN)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }

    return connfd;
}

int fswSocket_close(int fd)
{
    int ret;

    ret = close(fd);
    if (ret < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    return ret;
}

int fswSocket_shutdown(int sock, int how)
{
    int ret;

    ret = shutdown(sock, how);

    if (ret < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    return ret;
}

int fswSocket_listen(int sock, int backlog)
{
    int ret;

    ret = listen(sock, backlog);
    if (ret < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    return ret;
}

ssize_t fswSocket_recv(int sock, void *buf, size_t len, int flag)
{
    ssize_t ret;

    ret = recv(sock, buf, len, flag);
    if (ret < 0 && errno != EAGAIN)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    return ret;
}

ssize_t fswSocket_send(int sock, const void *buf, size_t len, int flag)
{
    ssize_t ret;

    ret = send(sock, buf, len, flag);
    if (ret < 0 && errno != EAGAIN)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    return ret;
}

int fswSocket_set_nonblock(int sock)
{
    int flags;

    flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        return -1;
    }
    flags = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    if (flags < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        return -1;
    }
    return 0;
}