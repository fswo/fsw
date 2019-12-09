#include "fsw/coroutine_server.h"
#include "fsw/coroutine.h"

using fsw::Coroutine;
using fsw::coroutine::Server;
using fsw::coroutine::Socket;
using fsw::coroutine::run;

char response_str[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: 11\r\n\r\nhello world\r\n";

void handler(void *args)
{
    int ret;
    char buf[1024] = {0};
    Socket *conn = (Socket *)args;
    
    ret = conn->recv(buf, sizeof(buf) - 1);
    if (ret < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    buf[ret] = 0;
    ret = conn->send(response_str, sizeof(response_str) - 1);
    if (ret < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    delete conn;
}

int main(int argc, char const *argv[])
{
    run([]()
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->set_handler(handler);
        serv->start();
    });

    return 0;
}