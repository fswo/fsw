# HTTP Server

create an HTTP server:

```cpp
fsw::coroutine::http::Server::Server(char *host, int port)
```

start the HTTP Server:

```cpp
bool fsw::coroutine::http::Server::start()
```

## custom handler

set the http handler:

```cpp
void fsw::coroutine::http::Server::set_http_handler(std::string pattern, on_accept_handler fn)
```

```cpp
#include "fsw/coroutine_http.h"
#include "fsw/coroutine_http_server.h"
#include "fsw/coroutine.h"
#include "fsw/buffer.h"

using fsw::Coroutine;
using fsw::coroutine::http::Request;
using fsw::coroutine::http::Response;
using fsw::coroutine::http::Server;
using fsw::Buffer;
using fsw::coroutine::run;

void handler(Request *request, Response *response)
{
    char response_body[] = "hello world";
    Buffer buffer(1024);
    buffer.append(response_body, sizeof(response_body) - 1);

    response->set_header("Content-Type", "text/html");
    response->end(&buffer);

    return;
}

int main(int argc, char const *argv[])
{
    run([](void *args)
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->set_http_handler("/index", handler);
        serv->start();
    });

    return 0;
}
```

## document root

```cpp
#include "fsw/coroutine_http_server.h"

using fsw::coroutine::http::Server;
using fsw::coroutine::run;

int main(int argc, char const *argv[])
{
    run([]()
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->settings["document_root"] = "/var/www/html";
        serv->start();
    });

    return 0;
}
```
