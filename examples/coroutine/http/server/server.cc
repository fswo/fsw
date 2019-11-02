#include "fsw/coroutine_http.h"
#include "fsw/coroutine_http_server.h"
#include "fsw/coroutine.h"
#include "fsw/buffer.h"

using fsw::Coroutine;
using fsw::coroutine::http::Request;
using fsw::coroutine::http::Response;
using fsw::coroutine::http::Server;
using fsw::coroutine::Socket;
using fsw::Buffer;

void handler(Request *request, Response *response)
{
    char header1[] = "Content-Type";
    Buffer buffer1(12);
    buffer1.append(header1, 12);
    char value1[] = "text/html";
    Buffer buffer2(9);
    buffer2.append(value1, 9);

    char header2[] = "Connection";
    Buffer buffer3(10);
    buffer3.append(header2, 10);
    char value2[] = "close";
    Buffer buffer4(5);
    buffer4.append(value2, 5);

    char header3[] = "Content-Length";
    Buffer buffer5(14);
    buffer5.append(header3, 14);
    char value3[] = "11";
    Buffer buffer6(2);
    buffer6.append(value3, 2);

    char response_body[] = "hello world";
    Buffer *buffer = new Buffer(1024);
    buffer->append(response_body, sizeof(response_body) - 1);

    response->set_header(&buffer1, &buffer2);
    response->set_header(&buffer3, &buffer4);
    response->set_header(&buffer5, &buffer6);
    response->end(buffer);

    delete buffer;
    return;
}

int main(int argc, char const *argv[])
{
    fsw_event_init();

    Coroutine::create([](void *arg)
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->set_handler(string("/index"), handler);
        serv->start();
    });

    fsw_event_wait();

    return 0;
}