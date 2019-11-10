#include "coroutine_http_server.h"
#include "coroutine_http.h"
#include "socket.h"
#include "log.h"
#include "coroutine.h"
#include "http_parser.h"
#include "buffer.h"

using fsw::coroutine::http::Request;
using fsw::coroutine::http::Response;
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

static bool call_http_handler(on_accept_handler handler, Ctx *ctx)
{
    if (ctx->request->has_sec_websocket_key())
    {
        ctx->response->send_bad_request_response("no support websocket");
        return false;
    }
    handler(ctx->request, ctx->response);
    return true;
}

static bool call_websocket_handler(on_accept_handler handler, Ctx *ctx)
{
    if (!ctx->request->has_sec_websocket_key())
    {
        ctx->response->send_bad_request_response("websocket handshake error: 'Sec-WebSocket-Key' header is missing or blank");
        return false;
    }
    handler(ctx->request, ctx->response);
    return true;
}

static void http_connection_on_accept(void *arg)
{
    ssize_t recved;
    /**
     * Note that the coroutine cannot be switched out, otherwise the member content in arg may change.
     */
    Server *server = ((http_accept_handler_args *)arg)->server;
    Socket *conn = ((http_accept_handler_args *)arg)->conn;
    Ctx *ctx = new Ctx(conn);
    Coroutine::defer([](void *arg)
    {
        Ctx *ctx = (Ctx *)arg;
        delete ctx;
    }, (void *)ctx);

    http_parser_init(&ctx->parser, HTTP_REQUEST);

    while (true)
    {
        on_accept_handler handler;

        recved = conn->recv(conn->get_read_buf()->c_buffer(), READ_BUF_MAX_SIZE);
        if (recved == 0)
        {
            break;
        }

        /* Start up / continue the parser.
        * Note we pass recved==0 to signal that EOF has been received.
        */
        ctx->parse(recved);
        string path(ctx->request->path);
        handler = server->get_http_handler(path);
        if (handler != nullptr)
        {
            if (!call_http_handler(handler, ctx))
            {
                break;
            }
        }
        else
        {
            handler = server->get_websocket_handler(path);
            if (handler != nullptr)
            {
                if (!call_websocket_handler(handler, ctx))
                {
                    break;
                }
            }
        }
        if (!ctx->keep_alive)
        {
            break;
        }
        ctx->clear();
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

void Server::set_http_handler(string pattern, on_accept_handler fn)
{
    set_handler(pattern, fn, &http_handlers);
}

void Server::set_websocket_handler(string pattern, on_accept_handler fn)
{
    set_handler(pattern, fn, &websocket_handlers);
}

void Server::set_handler(string pattern, on_accept_handler fn, std::map<std::string, on_accept_handler> *handlers)
{
    (*handlers)[pattern] = fn;
}

on_accept_handler Server::get_http_handler(string pattern)
{
    return get_handler(pattern, &http_handlers);
}

on_accept_handler Server::get_websocket_handler(string pattern)
{
    return get_handler(pattern, &websocket_handlers);
}

on_accept_handler Server::get_handler(string pattern, std::map<std::string, on_accept_handler> *handlers)
{
    for (auto i = handlers->begin(); i != handlers->end(); i++)
    {
        if (strncasecmp(i->first.c_str(), pattern.c_str(), i->first.length()) == 0 && i->first.length() == pattern.length())
        {
            return i->second;
        }
    }
    
    return nullptr;
}