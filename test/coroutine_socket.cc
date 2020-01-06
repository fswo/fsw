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
    ret = sock->bind(fsw::Socket::FSW_SOCK_TCP, ip, 6000);
    ASSERT_EQ(ret, true);
}

TEST(coroutine_socket, listen)
{
    int ret;
    char ip[] = "127.0.0.1";
    Socket *sock = new Socket(AF_INET, SOCK_STREAM, 0);
    sock->bind(fsw::Socket::FSW_SOCK_TCP, ip, 6001);
    ret = sock->listen(512);
    ASSERT_EQ(ret, true);
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

TEST(coroutine_socket, get_name)
{
    int ret;
    char ip[] = "127.0.0.1";
    Socket *sock = new Socket(AF_INET, SOCK_STREAM, 0);
    std::map<std::string, std::string> info;

    sock->bind(fsw::Socket::FSW_SOCK_TCP, ip, 6002);
    sock->listen(512);
    info = sock->get_name();
    auto it = info.find("address");
    ASSERT_EQ(it->second, "127.0.0.1");
    it = info.find("port");
    ASSERT_EQ(it->second, "6002");
}
