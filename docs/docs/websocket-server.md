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
#include "fsw/websocket_frame.h"

using namespace fsw::coroutine::http;

using fsw::Coroutine;
using fsw::coroutine::http::Request;
using fsw::coroutine::http::Response;
using fsw::coroutine::http::Server;
using fsw::Buffer;
using fsw::websocket::Frame;
using fsw::coroutine::run;

void websocket_handler(Request *request, Response *response)
{
    std::string data = "hello websocket";
    while (true)
    {
        Frame frame;
        response->recv_frame(&frame);
        std::string recv_data(frame.payload, frame.payload_length);
        std::cout << recv_data << std::endl;

        Buffer send_data(data.length());
        send_data.append(data);
        response->send_frame(&send_data);
        Coroutine::sleep(1);
    }

    return;
}

int main(int argc, char const *argv[])
{
    run([](void *arg)
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->set_websocket_handler("/websocket", websocket_handler);
        serv->start();
    });

    return 0;
}
```
