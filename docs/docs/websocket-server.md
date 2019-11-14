# Websocket Server

Websocket Server is based on HTTP Server, so we can upgrade the protocol on the basis of HTTP Server.

## set_websocket_handler

set websocket handler:

```cpp
void fsw::coroutine::http::Server::set_websocket_handler(std::string pattern, on_accept_handler fn)
```

## recv_frame

recevie the frame from websocket client:

```cpp
void fsw::coroutine::http::Response::recv_frame(fsw::websocket::Frame *frame)
```

## send_frame

send the frame to websocket client:

```cpp
void fsw::coroutine::http::Response::send_frame(fsw::Buffer *data)
```

## websocket example

```cpp
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

void websocket_handler(Request *request, Response *response)
{
    char response_body[] = "hello websocket";
    Buffer buffer(1024);
    buffer.append(response_body, sizeof(response_body) - 1);
    while (true)
    {
        Coroutine::sleep(1);
        response->send_frame(&buffer);
    }

    return;
}

int main(int argc, char const *argv[])
{
    fsw_event_init();

    Coroutine::create([](void *arg)
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->set_websocket_handler("/websocket", websocket_handler);
        serv->start();
    });

    fsw_event_wait();

    return 0;
}
```
