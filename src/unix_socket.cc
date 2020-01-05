#include "unix_socket.h"

using fsw::UnixSocket;
using fsw::Buffer;

UnixSocket::UnixSocket()
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket) < 0)
    {
        set_err();
    }
    worker_fd = socket[0];
    master_fd = socket[1];
}

ssize_t UnixSocket::recv(void *buf, size_t len)
{
    int ret;

    ret = fswSocket_recv(current_fd, buf, len, 0);
    if (ret < 0)
    {
        set_err();
    }
    
    return ret;
}

ssize_t UnixSocket::send(const void *buf, size_t len)
{
    int ret;

    ret = fswSocket_send(current_fd, buf, len, 0);
    if (ret < 0)
    {
        set_err();
    }
    
    return ret;
}

Buffer* UnixSocket::get_read_buf()
{
    if (!read_buf)
    {
        read_buf = new Buffer(READ_BUF_MAX_SIZE);
    }
    return read_buf;
}

Buffer* UnixSocket::get_write_buf()
{
    if (!write_buf)
    {
        write_buf = new Buffer(WRITE_BUF_MAX_SIZE);
    }
    return write_buf;
}

bool UnixSocket::close()
{
    if (fswSocket_close(current_fd) < 0)
    {
        set_err();
        return false;
    }
    
    return true;
}