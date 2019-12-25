#include <iostream>
#include "fsw/socket.h"
#include "fsw/coroutine_socket.h"
#include "gtest/gtest.h"

using fsw::coroutine::Socket;

TEST(coroutine_socket, Socket_domain_type_protocol)
{
    Socket *sock = new Socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GT(sock->get_fd(), 2);
}

TEST(coroutine_socket, Socket_fd)
{
    int fd = open("/dev/null", O_RDONLY);
    Socket *sock = new Socket(fd);
    ASSERT_EQ(sock->get_fd(), fd);
}

TEST(coroutine_socket, bind)
{
    int ret;
    char ip[] = "127.0.0.1";
    Socket *sock = new Socket(AF_INET, SOCK_STREAM, 0);
    ret = sock->bind(FSW_SOCK_TCP, ip, 6666);
    ASSERT_EQ(ret, 0);
}

TEST(coroutine_socket, listen)
{
    int ret;
    char ip[] = "127.0.0.1";
    Socket *sock = new Socket(AF_INET, SOCK_STREAM, 0);
    sock->bind(FSW_SOCK_TCP, ip, 6666);
    ret = sock->listen(512);
    ASSERT_EQ(ret, 0);
}

TEST(coroutine_socket, set_option)
{
    int ret;
    int on = 1;
    Socket *sock = new Socket(AF_INET, SOCK_STREAM, 0);
    ret = sock->set_option(SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    ASSERT_TRUE(ret);
}

TEST(coroutine_socket, get_option)
{
    int ret;
    int on = 1;
    int get;
    socklen_t len = sizeof(get);
    Socket *sock = new Socket(AF_INET, SOCK_STREAM, 0);
    ret = sock->set_option(SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    sock->get_option(SOL_SOCKET, SO_REUSEADDR, &get, &len);
    ASSERT_EQ(get, 1);
}

