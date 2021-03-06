# About

[![Build Status](https://travis-ci.org/fswo/fsw.svg?branch=dev)](https://travis-ci.org/fswo/fsw)

fsw is a coroutine component that we can use to quickly develop a high performance server, etc.

## Support

### Coroutine

- [x] [**coroutine library**](https://fswo.github.io/fsw/coroutine/)
- [x] [**channel**](https://fswo.github.io/fsw/channel/)
- [x] [**tcp server**](https://fswo.github.io/fsw/tcp-server/)
- [x] [**http server**](https://fswo.github.io/fsw/http-server/)
- [x] [**websocket server**](https://fswo.github.io/fsw/websocket-server/)
- [x] [**http2 client**](https://fswo.github.io/fsw/http2-client/)
- [x] [**process manager module**](https://fswo.github.io/fsw/process/)

### HTTP

## Documentation

full documentation is available on the [Documentation site](https://fswo.github.io/fsw/coroutine/).

## Get started quickly

### Build

#### Build fsw

```shell
cmake . && make && make install
```

#### A HTTP Server example

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

void http_handler(Request *request, Response *response)
{
    char response_body[] = "hello world";
    Buffer buffer(1024);
    buffer.append(response_body, sizeof(response_body) - 1);

    response->header["Content-Type"] = "text/html";
    response->end(&buffer);

    return;
}

int main(int argc, char const *argv[])
{
    run([]()
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->set_http_handler("/index", http_handler);
        serv->start();
    });

    return 0;
}
```

#### Build the example

```shell
~/codeDir/cppCode/fsw/example # g++ server.cc -lfsw
```

#### Pressure test

The machine configuration is one cpu core, 2G memory:

```shell
ab -c 1000 -n 1000000 -k 127.0.0.1/index

Concurrency Level:      1000
Time taken for tests:   35.721 seconds
Complete requests:      1000000
Failed requests:        0
Keep-Alive requests:    1000000
Total transferred:      101000000 bytes
HTML transferred:       13000000 bytes
Requests per second:    27994.62 [#/sec] (mean)
Time per request:       35.721 [ms] (mean)
Time per request:       0.036 [ms] (mean, across all concurrent requests)
Transfer rate:          2761.19 [Kbytes/sec] received
```

#### A HTTP2 Client example

```cpp
#include "fsw/coroutine_http2_client.h"
#include "fsw/coroutine.h"

using fsw::coroutine::http2::Client;
using fsw::coroutine::http2::Request;
using fsw::coroutine::http2::Response;
using fsw::coroutine::run;

int main(int argc, char const *argv[])
{
    run([]()
    {
        bool ret;
        Client h2c;
        Request req;
        Response rep;

        ret = h2c.connect("127.0.0.1", 80);
        if (!ret)
        {
            fswError("%s", h2c.sock->get_err_msg());
        }
        h2c.send_request(&req);
        rep = h2c.recv_reponse();
        std::cout << rep.body << std::endl;
        return 0;
    });
}
```
