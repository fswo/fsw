#include "coroutine_http.h"
#include "log.h"
#include "base64.h"
#include <openssl/sha.h>

using fsw::coroutine::http::Request;
using fsw::coroutine::http::Response;
using fsw::coroutine::http::Ctx;
using fsw::coroutine::Socket;
using fsw::websocket::Frame;

#define WEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

static int http_request_on_message_begin(http_parser *parser);
static int http_request_on_url(http_parser *parser, const char *at, size_t length);
static int http_request_on_status(http_parser *parser, const char *at, size_t length);
static int http_request_on_header_field(http_parser *parser, const char *at, size_t length);
static int http_request_on_header_value(http_parser *parser, const char *at, size_t length);
static int http_request_on_headers_complete(http_parser *parser);
static int http_request_on_body(http_parser *parser, const char *at, size_t length);
static int http_request_on_message_complete(http_parser *parser);

inline static std::string compute_accept_key(std::string sec_websocket_key)
{
    std::string origin = sec_websocket_key + WEBSOCKET_GUID;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char *)origin.c_str(), origin.length(), hash);
    std::string base64_origin = base64_encode(hash, SHA_DIGEST_LENGTH);
    return base64_origin;
}

inline void set_http_version(Ctx *ctx, http_parser *parser)
{
    ctx->request->version = parser->http_major * 100 + parser->http_minor;
}

inline void set_http_method(Ctx *ctx, http_parser *parser)
{
    switch (parser->method)
    {
    case HTTP_GET:
        ctx->request->method = "GET";
        break;
    case HTTP_POST:
        ctx->request->method = "POST";
        break;
    default:
        break;
    }
}

static int http_request_on_message_begin(http_parser *parser)
{
    fswTrace("http request on message begin");
    return 0;
}

static int http_request_on_url(http_parser *parser, const char *at, size_t length)
{
    Ctx *ctx = (Ctx *)parser->data;
    /**
     * because const char *at may be destroyed, so must copy to ctx->request.path
     */
    ctx->request->path = new char[length + 1]();
    memcpy(ctx->request->path, at, length);
    ctx->request->path_len = length;
    return 0;
}

static int http_request_on_status(http_parser *parser, const char *at, size_t length)
{
    fswTrace("http request on status");
    return 0;
}

static int http_request_on_header_field(http_parser *parser, const char *at, size_t length)
{
    Ctx *ctx = (Ctx *)parser->data;
    ctx->current_header_name = (char *) at;
    ctx->current_header_name_len = length;
    return 0;
}

static int http_request_on_header_value(http_parser *parser, const char *at, size_t length)
{
    Ctx *ctx = (Ctx *)parser->data;
    std::map<std::string, std::string> &headers = ctx->request->header;
    std::string header_name(ctx->current_header_name, ctx->current_header_name_len);
    std::string header_value(at, length);

    std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::tolower);
    headers[header_name] = header_value;
    
    return 0;
}

static int http_request_on_headers_complete(http_parser *parser)
{
    Ctx *ctx = (Ctx *)parser->data;
    set_http_version(ctx, parser);
    set_http_method(ctx, parser);
    ctx->keep_alive = http_should_keep_alive(parser);
    return 0;
}

static int http_request_on_body(http_parser *parser, const char *at, size_t length)
{
    Ctx *ctx = (Ctx *)parser->data;
    ctx->request->body = new char[length + 1]();
    memcpy(ctx->request->body, at, length);
    ctx->request->body_length = length;
    return 0;
}

static int http_request_on_message_complete(http_parser *parser)
{
    fswTrace("http request on message complete");
    return 0;
}

static const http_parser_settings parser_settings =
{
    .on_message_begin = http_request_on_message_begin,
    .on_url = http_request_on_url,
    .on_status = http_request_on_status,
    .on_header_field = http_request_on_header_field,
    .on_header_value = http_request_on_header_value,
    .on_headers_complete = http_request_on_headers_complete,
    .on_body = http_request_on_body,
    .on_message_complete = http_request_on_message_complete,
};

Request::Request()
{
    
}

Request::~Request()
{
    clear();
}

Response::Response()
{
    
}

void Response::recv_frame(Frame *frame)
{
    ssize_t recved;
    Socket *conn = ctx->conn;
    Buffer buf(READ_BUF_MAX_SIZE);
    Buffer *read_buf = conn->get_read_buf();
    read_buf->clear();
    recved = conn->recv(buf.c_buffer(), READ_BUF_MAX_SIZE);
    read_buf->append(buf.c_buffer(), recved);

    frame->decode(read_buf);
}

void Response::send_frame(Buffer *data)
{
    clear_write_buf();
    Buffer *write_buf = get_write_buf();

    Frame::encode(write_buf, data);
    send_response();
}

Response::~Response()
{
    clear_header();
}

std::string Response::get_status_message()
{
    switch (_status)
    {
    case 100:
        return "100 Continue";
    case 101:
        return "101 Switching Protocols";
    case 201:
        return "201 Created";
    case 202:
        return "202 Accepted";
    case 203:
        return "203 Non-Authoritative Information";
    case 204:
        return "204 No Content";
    case 205:
        return "205 Reset Content";
    case 206:
        return "206 Partial Content";
    case 207:
        return "207 Multi-Status";
    case 208:
        return "208 Already Reported";
    case 226:
        return "226 IM Used";
    case 300:
        return "300 Multiple Choices";
    case 301:
        return "301 Moved Permanently";
    case 302:
        return "302 Found";
    case 303:
        return "303 See Other";
    case 304:
        return "304 Not Modified";
    case 305:
        return "305 Use Proxy";
    case 307:
        return "307 Temporary Redirect";
    case 400:
        return "400 Bad Request";
    case 401:
        return "401 Unauthorized";
    case 402:
        return "402 Payment Required";
    case 403:
        return "403 Forbidden";
    case 404:
        return "404 Not Found";
    case 405:
        return "405 Method Not Allowed";
    case 406:
        return "406 Not Acceptable";
    case 407:
        return "407 Proxy Authentication Required";
    case 408:
        return "408 Request Timeout";
    case 409:
        return "409 Conflict";
    case 410:
        return "410 Gone";
    case 411:
        return "411 Length Required";
    case 412:
        return "412 Precondition Failed";
    case 413:
        return "413 Request Entity Too Large";
    case 414:
        return "414 Request URI Too Long";
    case 415:
        return "415 Unsupported Media Type";
    case 416:
        return "416 Requested Range Not Satisfiable";
    case 417:
        return "417 Expectation Failed";
    case 418:
        return "418 I'm a teapot";
    case 421:
        return "421 Misdirected Request";
    case 422:
        return "422 Unprocessable Entity";
    case 423:
        return "423 Locked";
    case 424:
        return "424 Failed Dependency";
    case 426:
        return "426 Upgrade Required";
    case 428:
        return "428 Precondition Required";
    case 429:
        return "429 Too Many Requests";
    case 431:
        return "431 Request Header Fields Too Large";
    case 500:
        return "500 Internal Server Error";
    case 501:
        return "501 Method Not Implemented";
    case 502:
        return "502 Bad Gateway";
    case 503:
        return "503 Service Unavailable";
    case 504:
        return "504 Gateway Timeout";
    case 505:
        return "505 HTTP Version Not Supported";
    case 506:
        return "506 Variant Also Negotiates";
    case 507:
        return "507 Insufficient Storage";
    case 508:
        return "508 Loop Detected";
    case 510:
        return "510 Not Extended";
    case 511:
        return "511 Network Authentication Required";
    case 200:
    default:
        return "200 OK";
    }
}

void Response::set_header(Buffer *_name, Buffer *_value)
{
    Buffer *name = _name->dup();
    Buffer *value = _value->dup();
    header[name] = value;
}

void Response::set_header(std::string _name, std::string _value)
{
    Buffer *name = new Buffer(_name.length());
    name->append(_name);
    Buffer *value = new Buffer(_value.length());
    value->append(_value);
    header[name] = value;
}

bool Response::update_header(Buffer *_name, Buffer *_value)
{
    std::map<fsw::Buffer *, fsw::Buffer *>::iterator i;
    for (i = header.begin(); i != header.end(); i++)
    {
        if (memcpy(i->first->c_buffer(), _name->c_buffer(), i->first->length()) == 0)
        {
            Buffer *value = _value->dup();
            i->second = value;
            return true;
        }
    }
    return false;
}

Response* Response::build_http_status_line()
{
    Buffer* buf = get_write_buf();
    buf->append("HTTP/1.1 ")->append(get_status_message())->append("\r\n");
    return this;
}

Response* Response::build_http_header(int body_length)
{
    Buffer* buf = get_write_buf();

    for(auto h : this->header)
    {
        buf->append(h.first)->append(": ")->append(h.second)->append("\r\n");
    }
    if (ctx->keep_alive)
    {
        buf->append("Connection: Keep-Alive\r\n");
        /**
         * note the addition of content-length, 
         * otherwise the client might not disconnect, for example, curl
         */
        buf->append("Content-Length: ")->append(body_length)->append("\r\n");
    }
    else
    {
        buf->append("Connection: Close\r\n");
        buf->append("Content-Length: ")->append(body_length)->append("\r\n");
    }
    buf->append("\r\n");
    return this;
}

Response* Response::build_http_body(Buffer *body)
{
    Buffer* buf = get_write_buf();
    if (body)
    {
        buf->append(body)->append("\r\n");
    }
    
    return this;
}

void Response::end(Buffer *body)
{
    size_t body_length = 0;

    clear_write_buf();

    if (body)
    {
        body_length = body->length();
    }

    build_http_status_line()
        ->build_http_header(body_length)
        ->build_http_body(body)
        ->send_response();
}

void Response::clear_header()
{
    for (auto i = header.begin(); i != header.end(); i++)
    {
        delete i->first;
        delete i->second;
    }
    header.clear();
}

bool Response::upgrade()
{
    std::string bad_handshake = "websocket handshake error: ";

    if (ctx->request->method != "GET")
    {
        ctx->response->send_bad_request_response(bad_handshake + "request method is not GET");
        return false;
    }
    if (!ctx->request->header_contain_value("connection", "upgrade"))
    {
        ctx->response->send_bad_request_response(bad_handshake + "'upgrade' token not found in 'Connection' header");
        return false;
    }
    if (!ctx->request->header_contain_value("upgrade", "websocket"))
    {
        ctx->response->send_bad_request_response(bad_handshake + "'websocket' token not found in 'Upgrade' header");
        return false;
    }
    if (!ctx->request->header_contain_value("sec-websocket-version", "13"))
    {
        ctx->response->send_bad_request_response(bad_handshake + "websocket: unsupported version: 13 not found in 'Sec-Websocket-Version' header");
        return false;
    }
    std::string sec_websocket_key = ctx->request->get_header("sec-websocket-key");
    if (sec_websocket_key.empty())
    {
        ctx->response->send_bad_request_response(bad_handshake + "'Sec-WebSocket-Key' header is missing or blank");
        return false;
    }
    
    ctx->response->set_status(101);
    ctx->response->set_header("Upgrade", "websocket");
    ctx->response->set_header("Connection", "Upgrade");

    std::string accept_key = compute_accept_key(sec_websocket_key);
    ctx->response->set_header("Sec-WebSocket-Accept", accept_key);
    ctx->response->end();

    return true;
}

Ctx::Ctx(Socket *_conn)
{
    request = new Request();
    response = new Response();
    conn = _conn;
    parser.data = this;
    response->ctx = this;
}

Ctx::~Ctx()
{
    /**
     * TODO: Perhaps we can design a protocol for TCP payload 
     * that tells the client server that it has sent data, 
     * so the server doesn't need to send the FIN segment.
     */
    conn->shutdown(SHUT_WR);
    /**
     * TODO: If the client never clsoe the connection, 
     * the coroutine will always be suspended, causing a memory leak. 
     * Therefore, we need to add a timeout judgment logic to determine the timeout 
     * in the time drive and then forcibly disconnect the connection.
     */
    conn->check_client_close();
    delete conn;
    delete response;
    delete request;
}

size_t Ctx::parse(ssize_t recved)
{
    size_t nparsed;
    Socket *conn = this->conn;

    nparsed = http_parser_execute(&parser, &parser_settings, conn->get_read_buf()->c_buffer(), recved);
    return nparsed;
}

void Ctx::clear()
{
    request->clear();
    response->clear_header();
}
