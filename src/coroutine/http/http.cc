#include "coroutine_http.h"
#include "log.h"

using fsw::coroutine::http::Request;
using fsw::coroutine::http::Response;
using fsw::coroutine::http::Ctx;
using fsw::coroutine::Socket;

static int http_request_on_message_begin(http_parser *parser);
static int http_request_on_url(http_parser *parser, const char *at, size_t length);
static int http_request_on_status(http_parser *parser, const char *at, size_t length);
static int http_request_on_header_field(http_parser *parser, const char *at, size_t length);
static int http_request_on_header_value(http_parser *parser, const char *at, size_t length);
static int http_request_on_headers_complete(http_parser *parser);
static int http_request_on_body(http_parser *parser, const char *at, size_t length);
static int http_request_on_message_complete(http_parser *parser);

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
    ctx->request.path = new char[length + 1]();
    memcpy(ctx->request.path, at, length);
    ctx->request.path_len = length;
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
    std::map<char *, char *> &headers = ctx->request.header;
    size_t header_len = ctx->current_header_name_len;
    char *header_name = new char[header_len + 1]();

    memcpy(header_name, ctx->current_header_name, header_len);
    for (size_t i = 0; i < header_len; i++)
    {
        header_name[i] = tolower(header_name[i]);
    }

    char *header_value = new char[length + 1]();
    memcpy(header_value, at, length);
    headers[header_name] = header_value;
    
    return 0;
}

static int http_request_on_headers_complete(http_parser *parser)
{
    Ctx *ctx = (Ctx *)parser->data;
    ctx->request.version = parser->http_major * 100 + parser->http_minor;

    switch (parser->method)
    {
    case HTTP_GET:
        ctx->request.method = "GET";
        break;
    case HTTP_POST:
        ctx->request.method = "POST";
        break;
    default:
        break;
    }
    return 0;
}

static int http_request_on_body(http_parser *parser, const char *at, size_t length)
{
    Ctx *ctx = (Ctx *)parser->data;
    ctx->request.body = new char[length + 1]();
    memcpy(ctx->request.body, at, length);
    ctx->request.body_length = length;
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
    if (path)
    {
        delete[] path;
        path = nullptr;
    }

    /**
     * delete header name and header value
     */
    for (auto i = header.begin(); i != header.end(); i++)
    {
        delete[] i->first;
        delete[] i->second;
    }
    delete[] body;
}

Response::Response()
{
    
}

Response::~Response()
{
    
}

void Response::build_http_header(Buffer* buf)
{
    buf->append("HTTP/1.1 200 OK\r\n");
    for(auto h : this->header)
    {
        buf->append(h.first);
        buf->append(": ");
        buf->append(h.second);
        buf->append("\r\n");
    }
    buf->append("\r\n");
}

void Response::build_http_body(Buffer* buf, Buffer *body)
{
    buf->append(body);
    buf->append("\r\n");
}

void Response::end(Buffer *body)
{
    Socket *conn = this->ctx->conn;

    Buffer* buf = conn->get_write_buf();
    buf->clear();
    build_http_header(buf);
    build_http_body(buf, body);
    conn->send(buf->c_buffer(), buf->length());
}

Ctx::Ctx(Socket *_conn)
{
    conn = _conn;
    parser.data = this;
    this->response.ctx = this;
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
}

size_t Ctx::parse(ssize_t recved)
{
    size_t nparsed;
    Socket *conn = this->conn;

    nparsed = http_parser_execute(&parser, &parser_settings, conn->get_read_buf()->c_buffer(), recved);
    return nparsed;
}
