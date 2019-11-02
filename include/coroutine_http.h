#ifndef COROUTINE_HTTP_H
#define COROUTINE_HTTP_H

#include "fsw.h"
#include "coroutine_socket.h"
#include "http_parser.h"
#include "buffer.h"

using fsw::coroutine::Socket;
using fsw::Buffer;

namespace fsw { namespace coroutine { namespace http {

class Ctx;

class Request
{
public:
    std::string method;
    int version;
    char *path = nullptr;
    uint32_t path_len;
    char *body = nullptr;
    size_t body_length;
    std::map<char*, char*> header;

    Request();
    ~Request();
    void clear_path();
    void clear_body();
    void clear_header();
};


class Response
{
public:
    Ctx *ctx;
    std::string method;
    int version;
    int status;
    std::string reason;
    std::map<Buffer*, Buffer*> header;

    Response();
    ~Response();
    void set_header(Buffer *_name, Buffer *_value);
    void build_http_header(Buffer* buf);
    void build_http_body(Buffer* buf, Buffer *body);
    void end(Buffer *body);
    void clear_header();
};

class Ctx
{
public:
    Socket *conn;
    http_parser parser;
    Request request;
    Response response;
    char *current_header_name;
    size_t current_header_name_len;
    bool keep_alive;

    Ctx(Socket *_conn);
    ~Ctx();
    size_t parse(ssize_t recved);
    void clear();
};
}
}
}

#endif	/* COROUTINE_HTTP_H */