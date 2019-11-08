#include "coroutine_socket.h"
#include "coroutine.h"
#include "socket.h"
#include "log.h"

using fsw::Coroutine;
using fsw::coroutine::Socket;

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

int Socket::bind(int type, char *host, int port)
{
    return fswSocket_bind(sockfd, type, host, port);
}

int Socket::listen(int backlog)
{
    return fswSocket_listen(sockfd, backlog);
}

Socket* Socket::accept()
{
    int connfd;

    do
    {
        connfd = fswSocket_accept(sockfd);
    } while (connfd < 0 && errno == EAGAIN && wait_event(FSW_EVENT_READ));

    return (new Socket(connfd));
}

ssize_t Socket::recv(void *buf, size_t len)
{
    int ret;

    do
    {
        ret = fswSocket_recv(sockfd, buf, len, 0);
    } while (ret < 0 && errno == EAGAIN && wait_event(FSW_EVENT_READ));
    
    return ret;
}

ssize_t Socket::send(const void *buf, size_t len)
{
    int ret;

    do
    {
        ret = fswSocket_send(sockfd, buf, len, 0);
    } while (ret < 0 && errno == EAGAIN && wait_event(FSW_EVENT_WRITE));
    
    return ret;
}

int Socket::close()
{
    return fswSocket_close(sockfd);
}

int Socket::shutdown(int how)
{
    return fswSocket_shutdown(sockfd, how);
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

    ev->events = event == FSW_EVENT_READ ? EPOLLIN : EPOLLOUT;
    ev->data.u64 = touint64(sockfd, id);

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
