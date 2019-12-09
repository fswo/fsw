#ifndef COROUTINE_HTTP_H
#define COROUTINE_HTTP_H

#include "fsw.h"
#include "coroutine_socket.h"
#include "http_parser.h"
#include "buffer.h"
#include "log.h"
#include "websocket_frame.h"

using fsw::coroutine::Socket;
using fsw::Buffer;
using fsw::websocket::Frame;

namespace fsw { namespace coroutine { namespace http {

class Request;
class Response;

class Ctx
{
public:
    /**
     * conn must allocate memory on the heap and be freed by Ctx's destructor function.
     */
    Socket *conn;
    http_parser parser;
    Request *request;
    Response *response;
    char *current_header_name;
    size_t current_header_name_len;
    bool keep_alive;

    Ctx(Socket *_conn);
    ~Ctx();
    size_t parse(ssize_t recved);
    void clear();
};

class Request
{
public:
    std::string method;
    int version;
    char *path = nullptr;
    uint32_t path_len;
    char *body = nullptr;
    size_t body_length;

    /**
     * header_name is converted to lowercase
     */
    std::map<std::string, std::string> header;

    Request();
    ~Request();

    inline bool has_header(std::string header_name)
    {
        return header.find(header_name) != header.end();
    }

    inline std::string get_header(std::string header_name)
    {
        auto it = header.find(header_name);
        if (it == header.end())
        {
            return "";
        }
        return it->second;
    }

    inline bool header_contain_value(std::string header_name, std::string header_value)
    {
        auto it = header.find(header_name);
        if (it == header.end())
        {
            return false;
        
        }

        std::string real_value_lowercase = it->second;
        std::transform(real_value_lowercase.begin(), real_value_lowercase.end(), real_value_lowercase.begin(), ::tolower);
        if (real_value_lowercase != header_value)
        {
            return false;
        }

        return true;
    }

    inline bool has_sec_websocket_key()
    {
        std::string key = "sec-websocket-key";
        return has_header(key);
    }

    inline void clear()
    {
        clear_path()->clear_header()->clear_body();
    }

    inline Request* clear_path()
    {
        delete[] path;
        path = nullptr;
        path_len = 0;
        return this;
    }

    inline Request* clear_header()
    {
        header.clear();
        return this;
    }

    inline Request* clear_body()
    {
        delete[] body;
        body = nullptr;
        body_length = 0;
        return this;
    }
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
    int _status = 200;
    std::string _reason;
    std::map<std::string, std::string> header;

    Response();
    ~Response();

    void recv_frame(Frame *frame);
    void send_frame(Buffer *data, uint8_t opcode = 1, uint8_t finish = 1);
    std::string get_status_message();

    void set_header(std::string name, std::string value);
    Response* build_http_status_line();
    Response* build_http_header(int body_length);
    Response* build_http_body(Buffer *body);
    void end(Buffer *body = nullptr);
    void clear_header();
    bool upgrade();

    inline void set_status(int status)
    {
        _status = status;
    }

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

    inline Buffer* get_write_buf()
    {
        return ctx->conn->get_write_buf();
    }

    inline void clear_write_buf()
    {
        Buffer* buf = get_write_buf();
        buf->clear();
    }

    inline ssize_t send_response()
    {
        Buffer *buf = get_write_buf();
        return ctx->conn->send(buf->c_buffer(), buf->length());
    }

    inline void send_not_found_response()
    {
        ctx->response->set_status(404);
        ctx->response->end();
    }

    inline void send_bad_request_response(std::string body)
    {
        Buffer bad_body(body.length());
        bad_body.append(body);
        
        ctx->response->set_status(400);
        ctx->response->end(&bad_body);
    }
};
}
}
}

#endif	/* COROUTINE_HTTP_H */