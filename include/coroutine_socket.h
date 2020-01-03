#ifndef COROUTINE_SOCKET_H
#define COROUTINE_SOCKET_H

#define READ_BUF_MAX_SIZE 1024
#define WRITE_BUF_MAX_SIZE 1024

#include "fsw.h"
#include "buffer.h"

using fsw::Buffer;

namespace fsw { namespace coroutine {
    
class Socket
{
private:
    int sockfd;
    int err_code = 0;
    const char *err_msg = "";
    Buffer *read_buf = nullptr;
    Buffer *write_buf = nullptr;
public:
    Socket(int domain, int type, int protocol);
    Socket(int fd);
    ~Socket();
    bool bind(int type, char *host, int port);
    bool listen(int backlog);
    Socket* accept();
    ssize_t recv(void *buf, size_t len);
    ssize_t recv_all(void *buf, size_t len);
    ssize_t send(const void *buf, size_t len);
    ssize_t send_all(const void *buf, size_t len);
    bool close();
    bool shutdown(int how);
    bool set_option(int level, int optname, const void *optval, socklen_t optlen);
    bool get_option(int level, int optname, void *optval, socklen_t *optlen);
    std::map<std::string, std::string> get_name();
    std::map<std::string, std::string> get_peername();
    Buffer* get_read_buf();
    Buffer* get_write_buf();
    bool wait_event(int event);

    inline int get_fd()
    {
        return sockfd;
    }

    inline void check_client_close()
    {
        while (recv(read_buf->c_buffer(), READ_BUF_MAX_SIZE) > 0){}
    }

    inline void set_err()
    {
        err_code = errno;
        err_msg = err_code ? strerror(err_code) : "";
    }

    inline int get_err_code()
    {
        return err_code;
    }

    inline const char *get_err_msg()
    {
        return err_msg;
    }
};
}
}



#endif	/* COROUTINE_SOCKET_H */