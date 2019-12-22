#include "coroutine_server.h"
#include "socket.h"
#include "log.h"
#include "coroutine.h"

using fsw::coroutine::Server;
using fsw::coroutine::Socket;
using fsw::Coroutine;

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

        Coroutine::create(std::bind(handler, (void *)conn));
    }
    return true;
}

bool Server::shutdown()
{
    running = false;
    return true;
}

void Server::set_handler(handle_func_t fn)
{
    handler = fn;
}

handle_func_t Server::get_handler()
{
    return handler;
}