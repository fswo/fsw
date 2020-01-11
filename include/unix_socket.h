#pragma once

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
    enum type
    {
        WORKER = 1,
        MASTER = 2,
    };

    fsw::Socket *current_sock; // use for current process

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

    inline fsw::Socket *get_socket(int type)
    {
        return type == WORKER ? worker_sock : master_sock;
    }

private:
    int socket[2];
    int worker_fd;
    int master_fd;
    fsw::Socket *worker_sock;
    fsw::Socket *master_sock;
    Buffer *read_buf = nullptr;
    Buffer *write_buf = nullptr;
    int err_code = 0;
    const char *err_msg = "";
};
}
