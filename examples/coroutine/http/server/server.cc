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

void http_handler(Request *request, Response *response)
{
    char response_body[] = "hello world";
    Buffer buffer(1024);
    buffer.append(response_body, sizeof(response_body) - 1);

    response->header["Content-Type"] = "text/html";
    response->end(&buffer);

    return;
}

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
    run([]()
    {
        char ip[] = "127.0.0.1";

        Server *serv = new Server(ip, 80);
        serv->settings["document_root"] = "/var/www/html";
        serv->set_http_handler("/index", http_handler);
        serv->set_websocket_handler("/websocket", websocket_handler);
        serv->start();
    });

    return 0;
}