#include "fsw/coroutine_http.h"
#include "fsw/coroutine_http_server.h"
#include "fsw/coroutine.h"
#include "fsw/buffer.h"

using namespace fsw::coroutine::http;

using fsw::Coroutine;
using fsw::coroutine::http::Request;
using fsw::coroutine::http::Response;
using fsw::coroutine::http::Server;
using fsw::Buffer;

void http_handler(Request *request, Response *response)
{
    char response_body[] = "hello world";
    Buffer buffer(1024);
    buffer.append(response_body, sizeof(response_body) - 1);

    response->set_header("Content-Type", "text/html");
    response->end(&buffer);

    return;
}

void websocket_handler(Request *request, Response *response)
{
    char response_body[] = "hello websocket";
    Buffer buffer(1024);
    buffer.append(response_body, sizeof(response_body) - 1);
    response->send_frame(&buffer);

    return;
}

int main(int argc, char const *argv[])
{
    fsw_event_init();

    Coroutine::create([](void *arg)
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->set_http_handler("/index", http_handler);
        serv->set_websocket_handler("/websocket", websocket_handler);
        serv->start();
    });

    fsw_event_wait();

    return 0;
}