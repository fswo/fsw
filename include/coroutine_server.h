#pragma once

#include "fsw.h"
#include "coroutine_socket.h"

typedef void (*handle_func_t)(void*);

namespace fsw { namespace coroutine {
class Server
{
private:
    Socket *socket;
    handle_func_t handler;
    bool running;
    
public:
    Server(char *host, int port);
    ~Server();
    bool start();
    bool shutdown();
    void set_handler(handle_func_t fn);
    handle_func_t get_handler();
};
}
}
