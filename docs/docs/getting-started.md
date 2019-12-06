# Getting-Started

## Build

### Build fsw

```shell
cmake . && make && make install
```

### A HTTP Server example

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

### Build the example

```shell
~/codeDir/cppCode/fsw/example # g++ server.cc -lfsw
```

### Pressure test

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
