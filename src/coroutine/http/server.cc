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
    if (!ctx->response->upgrade())
    {
        return false;
    }
    handler(ctx->request, ctx->response);
    return true;
}

void Server::on_accept(Socket* conn)
{
    ssize_t recved;
    /**
     * Note that the coroutine cannot be switched out, otherwise the member content in arg may change.
     */
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

        string static_file = get_document_absolute_path(path);
        if (!static_file.empty())
        {
            int read_fd;
            struct stat stat_buf;
            off_t offset = 0;

            read_fd = open(static_file.c_str(), O_RDONLY);
            fstat(read_fd, &stat_buf);

            ctx->response->end(nullptr, stat_buf.st_size);
            sendfile (conn->get_fd(), read_fd, &offset, stat_buf.st_size);
            close (read_fd);
        } else if ((handler = get_http_handler(path)) != nullptr)
        {
            if (!call_http_handler(handler, ctx))
            {
                break;
            }
        }
        else if ((handler = get_websocket_handler(path)) != nullptr)
        {
            if (!call_websocket_handler(handler, ctx))
            {
                break;
            }
        }
        else
        {
            ctx->response->send_not_found_response();
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
    int on = 1;
    socket = new Socket(AF_INET, SOCK_STREAM, 0);

    socket->set_option(SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (!socket->bind(FSW_SOCK_TCP, host, port))
    {
        fswWarn("Error has occurred: (errno %d) %s", socket->get_err_code(), socket->get_err_msg());
        return;
    }
    if (!socket->listen(512))
    {
        fswWarn("Error has occurred: (errno %d) %s", socket->get_err_code(), socket->get_err_msg());
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

        Coroutine::create(std::bind(&Server::on_accept, this, conn));
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