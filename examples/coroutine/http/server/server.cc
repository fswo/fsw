#include "fsw/coroutine_http_server.h"
#include "fsw/coroutine.h"

using fsw::Coroutine;
using fsw::coroutine::http::Server;
using fsw::coroutine::Socket;

char response_str[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: 11\r\n\r\nhello world\r\n";

void handler(void *args)
{
    return;
}

int main(int argc, char const *argv[])
{
    fsw_event_init();

    Coroutine::create([](void *arg)
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->set_handler("/index", handler);
        serv->start();
    });

    fsw_event_wait();

    return 0;
}