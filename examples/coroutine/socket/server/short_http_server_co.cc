#include <iostream>
#include "fsw/coroutine.h"
#include "fsw/socket.h"
#include "fsw/coroutine_socket.h"
#include "fsw/log.h"

using namespace fsw;
using namespace std;
using fsw::coroutine::Socket;

char response_str[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: 11\r\n\r\nhello world\r\n";

struct co_param
{
    int port;
};

int main(int argc, char const *argv[])
{
    fsw_event_init();

    co_param param = {80};

    Coroutine::create([](void *param)
    {
        co_param *_param = (co_param *)param;
        int port = _param->port;
        char ip[] = "127.0.0.1";

        Socket sock(AF_INET, SOCK_STREAM, 0);
        sock.bind(FSW_SOCK_TCP, ip, port);
        sock.listen(512);
    
        while (true)
        {
            Socket* conn = sock.accept();

            Coroutine::create([](void *_sock)
            {
                int ret;
                char buf[1024] = {0};

                Socket *conn = (Socket *)_sock;

                fswDebug("connfd[%d] recv.", conn->get_fd());
                ret = conn->recv(buf, sizeof(buf) - 1);
                if (ret < 0)
                {
                    fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
                }
                fswDebug("connfd[%d] recv success.", conn.get_fd());
                
                buf[ret] = 0;
                fswDebug("connfd[%d] send.", conn.get_fd());
                ret = conn->send(buf, sizeof(buf) - 1);
                if (ret < 0)
                {
                    fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
                }
                fswDebug("connfd[%d] send success.", conn.get_fd());
                delete conn;
            }, (void *)conn);
        }
    }, &param);

    fsw_event_wait();

    return 0;
}