#include "socket.h"
#include "log.h"

using fsw::Socket;

Socket::Socket(int domain, int type, int protocol)
{
    fd = ::socket(domain, type, protocol);
    if (socket < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
}

Socket::Socket(int _fd)
{
    fd = _fd;
}

Socket::~Socket()
{
    close();
}

bool Socket::bind(int type, char *host, int port)
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
        if (::bind(fd, (sockaddr *)&servaddr, sizeof(servaddr)) < 0)
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

bool Socket::listen(int backlog)
{
    return ::listen(fd, backlog) < 0 ? false : true;
}

int Socket::accept()
{
    int connfd;
    struct sockaddr_in sa;
    socklen_t len;

    len = sizeof(sa);
    connfd = ::accept(fd, (struct sockaddr *)&sa, &len);
    return connfd;
}

bool Socket::close()
{
    return ::close(fd) < 0 ? false : true;
}

bool Socket::shutdown(int how)
{
    return ::shutdown(fd, how) < 0 ? false : true;
}

bool Socket::set_option(int level, int optname, const void *optval, socklen_t optlen)
{
    return setsockopt(fd, level, optname, optval, optlen) < 0 ? false : true;
}

bool Socket::get_option(int level, int optname, void *optval, socklen_t *optlen)
{
    return getsockopt(fd, level, optname, optval, optlen) < 0 ? false : true;
}

bool Socket::getname(sockaddr *addr, socklen_t *len)
{
    return getsockname(fd, addr, len) < 0 ? false : true;
}

bool Socket::getpeername(sockaddr *addr, socklen_t *len)
{
    return ::getpeername(fd, addr, len) < 0 ? false : true;
}

ssize_t Socket::recv(void *buf, size_t len, int flag)
{
    return ::recv(fd, buf, len, flag);
}

ssize_t Socket::send(const void *buf, size_t len, int flag)
{
    return ::send(fd, buf, len, flag);
}

int Socket::set_nonblock()
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        return -1;
    }
    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return flags;
}
