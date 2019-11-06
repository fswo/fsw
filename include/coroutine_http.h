#ifndef COROUTINE_HTTP_H
#define COROUTINE_HTTP_H

#include "fsw.h"
#include "coroutine_socket.h"
#include "http_parser.h"
#include "buffer.h"
#include "log.h"

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

    /**
     * 100 => 1.0
     * 101 => 1.1
     * 200 => 2.0
     */
    int _version;
    int _status;
    std::string _reason;
    std::map<Buffer*, Buffer*> header;

    Response();
    ~Response();

    /**
     * in order to prevent the user from modifying the header, 
     * set_header needs to copy the passed parameters.
     * 
     * in order to improve performance, set_header does not check if header name is set,
     * so you can't use it to update the header value.
     * if you want to update the value of the header name, you should use update_header.
     */
    void set_header(Buffer *_name, Buffer *_value);

    /**
     * update_header is looked up based on the c_buffer in the header name, 
     * so need to iterate through all the headers.
     */
    bool update_header(Buffer *_name, Buffer *_value);
    void build_http_header(int body_length);
    void build_http_body(Buffer *body);
    void end(Buffer *body);
    void clear_header();

    inline void set_version(int version)
    {
        _version = version;
    }

    inline std::string get_real_version()
    {
        switch (_version)
        {
        case 100:
            return "1.0";
            break;
        case 101:
            return "1.1";
            break;
        case 200:
            return "2.0";
            break;
        default:
            snprintf(fsw_error, sizeof(fsw_error), "Unknown version: %d", _version);
            return fsw_error;
            break;
        }
    }
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