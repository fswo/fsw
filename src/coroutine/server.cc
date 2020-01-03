#include "coroutine_server.h"
#include "socket.h"
#include "log.h"
#include "coroutine.h"

using fsw::coroutine::Server;
using fsw::coroutine::Socket;
using fsw::Coroutine;

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