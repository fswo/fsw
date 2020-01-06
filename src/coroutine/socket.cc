#include "coroutine_socket.h"
#include "coroutine.h"
#include "socket.h"
#include "log.h"
#include "help.h"

using fsw::Coroutine;
using fsw::coroutine::Socket;
using fsw::event::FswG;

Socket::Socket(int domain, int type, int protocol)
{
    sockfd = fswSocket_create(domain, type, protocol);
    if (sockfd < 0)
    {
        return;
    }
    fswSocket_set_nonblock(sockfd);
}

Socket::Socket(int fd)
{
    sockfd = fd;
    fswSocket_set_nonblock(sockfd);
}

Socket::~Socket()
{
    delete read_buf;
    delete write_buf;
    close();
}

bool Socket::bind(int type, char *host, int port)
{
    if (fswSocket_bind(sockfd, type, host, port) < 0)
    {
        set_err();
        return false;
    }
    
    return true;
}

bool Socket::listen(int backlog)
{
    int success = fswSocket_listen(sockfd, backlog);
    if (!success)
    {
        set_err();
    }
    
    return success;
}

Socket* Socket::accept()
{
    int connfd;

    do
    {
        connfd = fswSocket_accept(sockfd);
    } while (connfd < 0 && errno == EAGAIN && wait_event(fsw::event::fswEvent_type::FSW_EVENT_READ));

    return (new Socket(connfd));
}

ssize_t Socket::recv(void *buf, size_t len)
{
    int ret;

    do
    {
        ret = fswSocket_recv(sockfd, buf, len, 0);
    } while (ret < 0 && errno == EAGAIN && wait_event(fsw::event::fswEvent_type::FSW_EVENT_READ));
    
    return ret;
}

ssize_t Socket::recv_all(void *buf, size_t len)
{
    ssize_t ret;
    ssize_t total = 0;

    while (true)
    {
        do {
            ret = fswSocket_recv(sockfd, (char *)buf + total, len - total, 0);
        } while (ret < 0 && errno == EAGAIN && wait_event(fsw::event::fswEvent_type::FSW_EVENT_READ));
        if (ret <= 0)
        {
            if (total == 0)
            {
                total = ret;
            }
            break;
        }
        total += ret;
        if ((size_t)total == len)
        {
            break;
        }
    }
    return total;
}

ssize_t Socket::send(const void *buf, size_t len)
{
    int ret;

    do
    {
        ret = fswSocket_send(sockfd, buf, len, 0);
    } while (ret < 0 && errno == EAGAIN && wait_event(fsw::event::fswEvent_type::FSW_EVENT_WRITE));
    
    return ret;
}

ssize_t Socket::send_all(const void *buf, size_t len)
{
    ssize_t ret;
    ssize_t total = 0;

    while (true)
    {
        do {
            ret = fswSocket_send(sockfd, (char *)buf + total, len - total, 0);
        } while (ret < 0 && errno == EAGAIN && wait_event(fsw::event::fswEvent_type::FSW_EVENT_READ));
        if (ret <= 0)
        {
            if (total == 0)
            {
                total = ret;
            }
            break;
        }
        total += ret;
        if ((size_t)total == len)
        {
            break;
        }
    }
    return total;
}


bool Socket::close()
{
    bool success = fswSocket_close(sockfd);

    if (!success)
    {
        set_err();
    }
    return success;
}

bool Socket::shutdown(int how)
{
    bool success = fswSocket_shutdown(sockfd, how);

    if (!success)
    {
        set_err();
    }
    return success;
}

bool Socket::set_option(int level, int optname, const void *optval, socklen_t optlen)
{
    bool success = fswSocket_set_option(sockfd, level, optname, optval, optlen);

    if (!success)
    {
        set_err();
    }
    return success;
}

bool Socket::get_option(int level, int optname, void *optval, socklen_t *optlen)
{
    bool success = fswSocket_get_option(sockfd, level, optname, optval, optlen);

    if (!success)
    {
        set_err();
    }
    return success;
}

std::map<std::string, std::string> Socket::get_name()
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    std::map<std::string, std::string> info;

    if (fswSocket_getname(sockfd, (struct sockaddr *)&addr, &len))
    {
        info["address"] = inet_ntoa(addr.sin_addr);
        info["port"] = std::to_string(ntohs(addr.sin_port));
    }
    
    return info;
}

std::map<std::string, std::string> Socket::get_peername()
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    std::map<std::string, std::string> info;

    if (fswSocket_getpeername(sockfd, (struct sockaddr *)&addr, &len))
    {
        info["address"] = inet_ntoa(addr.sin_addr);
        info["port"] = std::to_string(ntohs(addr.sin_port));
    }
    
    return info;
}

Buffer* Socket::get_read_buf()
{
    if (!read_buf)
    {
        read_buf = new Buffer(READ_BUF_MAX_SIZE);
    }
    return read_buf;
}

Buffer* Socket::get_write_buf()
{
    if (!write_buf)
    {
        write_buf = new Buffer(WRITE_BUF_MAX_SIZE);
    }
    return write_buf;
}

bool Socket::wait_event(int event)
{
    long id;
    Coroutine* co;
    epoll_event *ev;

    co = Coroutine::get_current();
    id = co->get_cid();

    if (!FswG.poll)
    {
        fswError("Need to call fsw_event_init first.");
    }
    ev = FswG.poll->events;

    ev->events = event == fsw::event::fswEvent_type::FSW_EVENT_READ ? EPOLLIN : EPOLLOUT;
    ev->data.u64 = fsw::help::touint64(sockfd, id);

    fswTrace("add sockfd[%d] %s event", sockfd, "EPOLL_CTL_ADD");
    if (epoll_ctl(FswG.poll->epollfd, EPOLL_CTL_ADD, sockfd, ev) < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        return false;
    }
    (FswG.poll->event_num)++;

    Coroutine::yield();;

    fswTrace("remove sockfd[%d] %s event", sockfd, "EPOLL_CTL_DEL");

    if (epoll_ctl(FswG.poll->epollfd, EPOLL_CTL_DEL, sockfd, NULL) < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        return false;
    }

    return true;
}
