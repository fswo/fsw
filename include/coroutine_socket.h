#ifndef COROUTINE_SOCKET_H
#define COROUTINE_SOCKET_H

#define READ_BUF_MAX_SIZE 80 * 1024
#define WRITE_BUF_MAX_SIZE 80 * 1024

#include "fsw.h"
#include "buffer.h"

using fsw::Buffer;

namespace fsw { namespace coroutine {
    
class Socket
{
private:
    int sockfd;
    Buffer *read_buf = nullptr;
    Buffer *write_buf = nullptr;
public:
    Socket(int domain, int type, int protocol);
    Socket(int fd);
    ~Socket();
    int bind(int type, char *host, int port);
    int listen(int backlog);
    Socket* accept();
    ssize_t recv(void *buf, size_t len);
    ssize_t send(const void *buf, size_t len);
    int close();
    Buffer* get_read_buf();
    Buffer* get_write_buf();

    bool wait_event(int event);

    inline int get_fd()
    {
        return sockfd;
    }
};
}
}



#endif	/* COROUTINE_SOCKET_H */