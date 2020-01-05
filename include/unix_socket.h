#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

#define READ_BUF_MAX_SIZE 1024
#define WRITE_BUF_MAX_SIZE 1024

#include "fsw.h"
#include "buffer.h"
#include "socket.h"

namespace fsw
{
class UnixSocket
{
public:
    UnixSocket();
    ~UnixSocket();
    ssize_t recv(void *buf, size_t len);
    ssize_t send(const void *buf, size_t len);
    Buffer* get_read_buf();
    Buffer* get_write_buf();
    bool close();

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
private:
    int socket[2];
    int worker_fd;
    int master_fd;
    int current_fd = -1; // use for current process
    Buffer *read_buf = nullptr;
    Buffer *write_buf = nullptr;
    int err_code = 0;
    const char *err_msg = "";
};
}

#endif	/* UNIX_SOCKET_H */