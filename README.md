# Introduction

[![Build Status](https://travis-ci.org/fswo/fsw.svg?branch=dev)](https://travis-ci.org/fswo/fsw)

fsw is a coroutine component that we can use to quickly develop a high performance server, etc.

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
```

#### Build the example

```shell
~/codeDir/cppCode/fsw/example # g++ server.cc -lfsw
```

#### Pressure test

The machine configuration is one cpu core, 2G memory:

```shell
Concurrency Level:      100
Time taken for tests:   0.321 seconds
Complete requests:      10000
Failed requests:        0
Keep-Alive requests:    10000
Total transferred:      1010000 bytes
HTML transferred:       130000 bytes
Requests per second:    31171.10 [#/sec] (mean)
Time per request:       3.208 [ms] (mean)
Time per request:       0.032 [ms] (mean, across all concurrent requests)
Transfer rate:          3074.49 [Kbytes/sec] received
```

## docs

### Coroutine

create a coroutine and run it immediately:

```cpp
long Coroutine::create(coroutine_func_t fn, void * args)
```

get the currently running coroutine:

```cpp
static fsw::Coroutine *fsw::Coroutine::get_current()
```

swap out the current coroutine:

```cpp
void fsw::Coroutine::yield()
```

swap in the specified coroutine:

```cpp
void fsw::Coroutine::resume()
```

delayed execution of tasks:

```cpp
void fsw::Coroutine::defer(coroutine_func_t _fn, void *_args = (void *)nullptr)
```

let the current coroutine sleep:

```cpp
static int fsw::Coroutine::sleep(double seconds)
```

### Channel

create a channel:

```cpp
fsw::coroutine::Channel::Channel(size_t _capacity = 1UL)
```

push the data into the channel:

```cpp
bool fsw::coroutine::Channel::push(void *data, double timeout = (-1.0))
```

pop the data from the channel:

```cpp
void *fsw::coroutine::Channel::pop(double timeout = (-1.0))
```

### TCP Server

create an TCP server:

```cpp
Server::Server(char * host, int port)
```

start the TCP Server:

```cpp
bool Server::start()
```

set the handler to handle the connection

```cpp
void Server::set_handler(string pattern, on_accept_handler fn)
```

### HTTP Server

create an HTTP server:

```cpp
fsw::coroutine::http::Server::Server(char *host, int port)
```

start the HTTP Server:

```cpp
bool fsw::coroutine::http::Server::start()
```

set the router:

```cpp
void fsw::coroutine::http::Server::set_handler(std::string pattern, on_accept_handler fn)
```
