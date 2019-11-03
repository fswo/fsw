# Introduction

[![Build Status](https://travis-ci.org/fswo/fsw.svg?branch=dev)](https://travis-ci.org/fswo/fsw)

fsw is a coroutine component that we can use to quickly develop a high performance server, etc.

## Get started quickly

### Build

#### Build fsw

```shell
~/codeDir/cppCode/fsw # cmake .
~/codeDir/cppCode/fsw # make
~/codeDir/cppCode/fsw # make install
```

#### Build example

```shell
~/codeDir/cppCode/fsw/example # g++ example.cc -lfsw
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
