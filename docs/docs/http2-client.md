# HTTP2 Client

create an HTTP2 Client:

```cpp
fsw::coroutine::http2::Client()
```

## connect

connect the http2 server:

```cpp
bool fsw::coroutine::http2::Client::connect(std::string host, int port)
```

## send_request

send request to http2 server, and return the current stream_id:

```cpp
int32_t fsw::coroutine::http2::Client::send_request(fsw::coroutine::http2::Request *req)
```

## write_data

write data to the specified stream, send it to http2 server and return the current stream_id:

```cpp
int32_t fsw::coroutine::http2::Client::write_data(int32_t stream_id, char *data, size_t data_len, bool end_stream = false)
```

example:

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
        int32_t stream_id;

        ret = h2c.connect("127.0.0.1", 80);
        if (!ret)
        {
            fswError("%s", h2c.sock->get_err_msg());
        }

        req.method = "GET";
        req.path = "/";
        req.pipeline = true;

        req.header["host"] = "127.0.0.1";
        req.header["user-agent"] = "Chrome/49.0.2587.3";
        req.header["content-type"] = "text/html";

        req.body = (char *) "http2 client";
        req.body_length = strlen(req.body);

        stream_id = h2c.send_request(&req);
        std::cout << stream_id << std::endl;

        stream_id = h2c.write_data(stream_id, req.body, req.body_length, false);
        std::cout << stream_id << std::endl;

        stream_id = h2c.write_data(stream_id, req.body, req.body_length, true);
        std::cout << stream_id << std::endl;

        rep = h2c.recv_reponse();
        std::cout << rep.body << std::endl;

        return 0;
    });
}
```

## recv_reponse

receive the response from http2 server:

```cpp
fsw::coroutine::http2::Response fsw::coroutine::http2::Client::recv_reponse()
```

example:

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
        int32_t stream_id;

        ret = h2c.connect("127.0.0.1", 80);
        if (!ret)
        {
            fswError("%s", h2c.sock->get_err_msg());
        }

        req.method = "GET";
        req.path = "/";

        req.header["host"] = "127.0.0.1";
        req.header["user-agent"] = "Chrome/49.0.2587.3";
        req.header["content-type"] = "text/html";

        req.body = (char *) "http2 client";
        req.body_length = strlen(req.body);

        stream_id = h2c.send_request(&req);
        std::cout << stream_id << std::endl;
        rep = h2c.recv_reponse();
        std::cout << rep.body << std::endl;

        return 0;
    });
}
```
