#include "coroutine_http_server.h"
#include "coroutine_http.h"
#include "socket.h"
#include "log.h"
#include "coroutine.h"
#include "http_parser.h"
#include "buffer.h"

using fsw::coroutine::http::Request;
using fsw::coroutine::http::Server;
using fsw::coroutine::http::Ctx;
using fsw::coroutine::Socket;
using fsw::Coroutine;
using fsw::Buffer;

struct http_accept_handler_args
{
    Server *server;
    Socket *conn;
};

static void http_connection_on_accept(void *arg)
{
    ssize_t recved;
    /**
     * Note that the coroutine cannot be switched out, otherwise the member content in arg may change.
     */
    Server *server = ((http_accept_handler_args *)arg)->server;
    Socket *conn = ((http_accept_handler_args *)arg)->conn;
    Ctx *ctx = new Ctx(conn);
    Coroutine *co = Coroutine::get_current();
    co->defer([](void *arg)
    {
        Ctx *ctx = (Ctx *)arg;
        delete ctx;
    }, (void *)ctx);

    http_parser_init(&ctx->parser, HTTP_REQUEST);

    recved = conn->recv(conn->get_read_buf()->c_buffer(), READ_BUF_MAX_SIZE);
    if (recved == 0)
    {
        return;
    }

    /* Start up / continue the parser.
    * Note we pass recved==0 to signal that EOF has been received.
    */
    ctx->parse(recved);
    string path(ctx->request.path);
    on_accept_handler handler = server->get_handler(path);
    if (handler != nullptr)
    {
        handler(&(ctx->request), &(ctx->response));
    }
}

Server::Server(char *host, int port)
{
    socket = new Socket(AF_INET, SOCK_STREAM, 0);
    if (socket->bind(FSW_SOCK_TCP, host, port) < 0)
    {
        fswWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        return;
    }
    if (socket->listen(512) < 0)
    {
        return;
    }
}

Server::~Server()
{
}

bool Server::start()
{
    running = true;

    while (running)
    {
        Socket* conn = socket->accept();
        if (!conn)
        {
            return false;
        }

        http_accept_handler_args arg = {this, conn};
        Coroutine::create(http_connection_on_accept, (void *)&arg);
    }
    return true;
}

bool Server::shutdown()
{
    running = false;
    return true;
}

void Server::set_handler(string pattern, on_accept_handler fn)
{
    handlers[pattern] = fn;
}

on_accept_handler Server::get_handler(string pattern)
{
    for (auto i = handlers.begin(); i != handlers.end(); i++)
    {
        if (strncasecmp(i->first.c_str(), pattern.c_str(), i->first.length()) == 0 && i->first.length() == pattern.length())
        {
            return i->second;
        }
    }
    return nullptr;
}