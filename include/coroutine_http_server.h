#ifndef COROUTINE_HTTP_SERVER_H
#define COROUTINE_HTTP_SERVER_H

#include "fsw.h"
#include "coroutine.h"
#include "coroutine_socket.h"
#include "coroutine_http.h"

using namespace std;
using fsw::coroutine::http::Request;
using fsw::coroutine::http::Response;

typedef void (*on_accept_handler)(Request*, Response*);

namespace fsw { namespace coroutine { namespace http {
class Server
{  
public:
    enum handler_type
    {
        HTTP = 1,
        WEBSOCKET = 2,
    };

    Server(char *host, int port);
    ~Server();
    bool start();
    bool shutdown();
    void set_http_handler(string pattern, on_accept_handler fn);
    void set_websocket_handler(string pattern, on_accept_handler fn);
    on_accept_handler get_http_handler(string pattern);
    on_accept_handler get_websocket_handler(string pattern);

private:
    Socket *socket;
    map<string, on_accept_handler> http_handlers;
    map<string, on_accept_handler> websocket_handlers;
    bool running;

    void set_handler(string pattern, on_accept_handler fn, std::map<std::string, on_accept_handler> *handlers);
    on_accept_handler get_handler(string pattern, std::map<std::string, on_accept_handler> *http_handlers);
    void on_accept(Socket* conn);
};
}
}
}

#endif	/* COROUTINE_HTTP_SERVER_H */