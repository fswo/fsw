#include "coroutine_http2_client.h"
#include "event.h"
#include "log.h"
#include "socket.h"
#include "buffer.h"

using fsw::FswG;
using fsw::Buffer;
using fsw::coroutine::http2::Frame;
using fsw::coroutine::http2::Client;
using fsw::coroutine::http2::Headers;
using fsw::coroutine::http2::Response;

Client::Client()
{
    int ret;

    stream_id = 1;
    init_settings_local_settings();
    init_settings_remote_settings();

    ret = nghttp2_hd_deflate_new(&deflater, FSW_HTTP2_DEFAULT_HEADER_TABLE_SIZE);
    if (ret != 0)
    {
        fswError("nghttp2_hd_deflate_new() failed with error: %s", nghttp2_strerror(ret));
    }

    ret = nghttp2_hd_inflate_new(&inflater);
    if (ret != 0)
    {
        fswError("nghttp2_hd_inflate_new() failed with error: %s", nghttp2_strerror(ret));
    }
}

void Client::init_settings_local_settings()
{
    local_settings.header_table_size = FSW_HTTP2_DEFAULT_HEADER_TABLE_SIZE;
    local_settings.window_size = FSW_HTTP2_DEFAULT_WINDOW_SIZE;
    local_settings.max_concurrent_streams = FSW_HTTP2_MAX_MAX_CONCURRENT_STREAMS;
    local_settings.max_frame_size = FSW_HTTP2_MAX_MAX_FRAME_SIZE;
    local_settings.max_header_list_size = FSW_HTTP2_DEFAULT_MAX_HEADER_LIST_SIZE;
}

void Client::init_settings_remote_settings()
{
    remote_settings.header_table_size = FSW_HTTP2_DEFAULT_HEADER_TABLE_SIZE;
    remote_settings.window_size = FSW_HTTP2_DEFAULT_WINDOW_SIZE;
    remote_settings.max_concurrent_streams = FSW_HTTP2_MAX_MAX_CONCURRENT_STREAMS;
    remote_settings.max_frame_size = FSW_HTTP2_MAX_MAX_FRAME_SIZE;
    remote_settings.max_header_list_size = FSW_HTTP2_DEFAULT_MAX_HEADER_LIST_SIZE;
}

ssize_t Client::build_http_header(Frame *frame, Request *req)
{
    char *buffer = frame->payload;

    Headers headers(8 + req->header.size());
    headers.add(FSW_STRL(":method"), req->method.c_str(), req->method.length());
    headers.add(FSW_STRL(":path"), req->path.c_str(), req->path.length());
    headers.add(FSW_STRL(":scheme"), FSW_STRL("http"));
    headers.add(FSW_STRL(":authority"), FSW_STRL("127.0.0.1"));

    for (auto iter = req->header.begin( ); iter != req->header.end( ); iter++)
    {
        headers.add(FSW_STRL(iter->first.c_str()), FSW_STRL(iter->second.c_str()));
    }

    size_t buflen = nghttp2_hd_deflate_bound(deflater, headers.get(), headers.len());
    ssize_t rv = nghttp2_hd_deflate_hd(deflater, (uint8_t *) buffer, buflen, headers.get(), headers.len());
    if (rv < 0)
    {
        fswError("Error has occurred: (errno %zu) %s", rv, "nghttp2_hd_deflate_hd() failed");
        return FSW_ERR;
    }
    return rv;
}

fswReturn_code Client::parse_frame(Frame *frame)
{
    if (stream_id > last_stream_id)
    {
        last_stream_id = stream_id;
    }

    int value = 0;

    switch (frame->type)
    {
    case FSW_HTTP2_TYPE_SETTINGS:
        value = parse_setting_frame(frame);
        return FSW_CONTINUE;
    case FSW_HTTP2_TYPE_WINDOW_UPDATE:
        return FSW_CONTINUE;
    case FSW_HTTP2_TYPE_PING:
        return FSW_CONTINUE;
    case FSW_HTTP2_TYPE_GOAWAY:
        return FSW_CONTINUE;
    case FSW_HTTP2_TYPE_RST_STREAM:
        return FSW_CONTINUE;
    case FSW_HTTP2_TYPE_PUSH_PROMISE:
        return FSW_CONTINUE;
    default:
        break;
    }

    if (frame->type == FSW_HTTP2_TYPE_HEADERS)
    {
        parse_frame_header(frame);
    }
    else if (frame->type == FSW_HTTP2_TYPE_DATA)
    {
        parse_payload(frame);
    }

    bool end = (frame->flags & FSW_HTTP2_FLAG_END_STREAM)   ||
            frame->type == FSW_HTTP2_TYPE_RST_STREAM        ||
            frame->type == FSW_HTTP2_TYPE_GOAWAY;
    if (end)
    {
        if (frame->type == FSW_HTTP2_TYPE_RST_STREAM)
        {
            frame->stream->response.status_code = -3; /* HTTP_CLIENT_ESTATUS_SERVER_RESET */
            frame->stream->response.err_code = value;
        }
        if (frame->stream->buffer && frame->stream->buffer->length() > 0)
        {
            frame->stream->response.body = std::string(frame->stream->buffer->c_buffer(), frame->stream->buffer->length());
            frame->stream->buffer->clear();
        }
        return FSW_READY;
    }

    return FSW_CONTINUE;
}

bool Client::parse_header_stop(int inflate_flags, ssize_t inlen)
{
    bool stop = inflate_flags & NGHTTP2_HD_INFLATE_FINAL;
    if (stop)
    {
        nghttp2_hd_inflate_end_headers(inflater);
    }
    stop = stop || (inlen == 0);

    return stop;
}

void Client::build_setting_frame(Frame *frame)
{
    uint16_t id = 0;
    uint32_t value = 0;
    Buffer *write_buf = sock->get_write_buf();
    char *buf = write_buf->c_buffer();

    frame->flags = 0;
    frame->type = FSW_HTTP2_TYPE_SETTINGS;
    frame->stream_id = 0;
    frame->payload = buf + FSW_HTTP2_FRAME_HEADER_SIZE;
    frame->payload_length = 3 * 6;
    build_frame_header(frame);

    char *p = frame->payload;

    /**
     * header table size
     */
    id = htons(FSW_HTTP2_SETTING_HEADER_TABLE_SIZE);
    memcpy(p, &id, sizeof(id));
    p += 2;
    value = htonl(local_settings.header_table_size);
    memcpy(p, &value, sizeof(value));
    p += 4;
    /**
     * max concurrent streams
     */
    id = htons(FSW_HTTP2_SETTINGS_MAX_CONCURRENT_STREAMS);
    memcpy(p, &id, sizeof(id));
    p += 2;
    value = htonl(local_settings.max_concurrent_streams);
    memcpy(p, &value, sizeof(value));
    p += 4;
    /**
     * init window size
     */
    id = htons(FSW_HTTP2_SETTINGS_INIT_WINDOW_SIZE);
    memcpy(p, &id, sizeof(id));
    p += 2;
    value = htonl(local_settings.window_size);
    memcpy(p, &value, sizeof(value));
    p += 4;
}

/**
 * serialize the Frame structure as byte stream
 */
void Client::build_frame_header(Frame *frame)
{
    char *buf = frame->payload - FSW_HTTP2_FRAME_HEADER_SIZE;

    buf[0] = frame->payload_length >> 16;
    buf[1] = frame->payload_length >> 8;
    buf[2] = frame->payload_length;
    buf[3] = frame->type;
    buf[4] = frame->flags;
    *(uint32_t *) (buf + 5) = htonl(frame->stream_id);
}

int Client::parse_frame_header(Frame *frame)
{
    char *in = frame->payload;
    ssize_t inlen = frame->payload_length;
    Response response = frame->stream->response;

    ssize_t rv;
    int inflate_flags = 0;
    while (!parse_header_stop(inflate_flags, inlen))
    {
        nghttp2_nv nv;

        rv = nghttp2_hd_inflate_hd(inflater, &nv, &inflate_flags, (uint8_t *) in, inlen, 1);
        if (rv < 0)
        {
            return FSW_ERR;
        }

        in += (size_t) rv;
        inlen -= (size_t) rv;

        if (inflate_flags & NGHTTP2_HD_INFLATE_EMIT)
        {
            if (nv.name[0] == ':')
            {
                if ((nv.namelen - 1 == strlen("status")) && (strncasecmp((char *) nv.name + 1, "status", nv.namelen - 1) == 0))
                {
                    response.status_code = atoi((char *) nv.value);
                    continue;
                }
            }
            response.header[(char *) nv.name] = (char *) nv.value;
        }
    }

    return FSW_OK;
}

void Client::parse_payload(Frame *frame)
{
    if (!(frame->flags & FSW_HTTP2_FLAG_END_STREAM))
    {
        frame->stream->flags |= FSW_HTTP2_STREAM_PIPELINE_RESPONSE;
    }
    if (frame->payload_length > 0)
    {
        if (!frame->stream->buffer)
        {
            frame->stream->buffer = new Buffer(READ_BUF_MAX_SIZE);
        }
        frame->stream->buffer->append(frame->payload, frame->payload_length);
        local_settings.window_size -= frame->payload_length;
        frame->stream->local_window_size -= frame->payload_length;
    }
}

int Client::parse_setting_frame(Frame *frame)
{
    uint16_t id = 0;
    uint32_t value = 0;
    char *buf = frame->payload;
    ssize_t payload_length = frame->payload_length;

    char setting_ack_buf[FSW_HTTP2_FRAME_HEADER_SIZE + FSW_HTTP2_FRAME_PING_PAYLOAD_SIZE];

    while (payload_length > 0)
    {
        id = ntohs(*(uint16_t *) (buf));
        value = ntohl(*(uint32_t *) (buf + sizeof(uint16_t)));
        switch (id)
        {
        case FSW_HTTP2_SETTING_HEADER_TABLE_SIZE:
            if (value != remote_settings.header_table_size)
            {
                remote_settings.header_table_size = value;
                int ret = nghttp2_hd_deflate_change_table_size(deflater, value);
                if (ret != 0)
                {
                    return FSW_ERR;
                }
            }
            break;
        case FSW_HTTP2_SETTINGS_MAX_CONCURRENT_STREAMS:
            remote_settings.max_concurrent_streams = value;
            break;
        case FSW_HTTP2_SETTINGS_INIT_WINDOW_SIZE:
            remote_settings.window_size = value;
            break;
        case FSW_HTTP2_SETTINGS_MAX_FRAME_SIZE:
            remote_settings.max_frame_size = value;
            break;
        case FSW_HTTP2_SETTINGS_MAX_HEADER_LIST_SIZE:
            remote_settings.max_header_list_size = value;
            break;
        default:
            break;
        }
        buf += sizeof(id) + sizeof(value);
        payload_length -= sizeof(id) + sizeof(value);
    }
    set_frame_header(setting_ack_buf, FSW_HTTP2_TYPE_SETTINGS, 0, FSW_HTTP2_FLAG_ACK, frame->stream_id);
    if (!send(setting_ack_buf, FSW_HTTP2_FRAME_HEADER_SIZE))
    {
        return FSW_ERR;
    }
    return value;
}

bool Client::connect(std::string host, int port)
{
    if (sock != nullptr)
    {
        return true;
    }

    sock = new Socket(AF_INET, SOCK_STREAM, 0);

    if (!sock->connect(host, port))
    {
        return false;
    }

    /**
     * send http2 pri
     */
    if (!send(FSW_STRL(FSW_HTTP2_PRI_STRING)))
    {
        return false;
    }

    /**
     * send http2 setting frame
     */
    Frame frame;
    build_setting_frame(&frame);
    return send_frame(&frame);
}

bool Client::send_http_header_frame(Frame *frame, Request *req)
{
    Stream *stream = new Stream(stream_id, false);
    frame->payload = FswG.buffer_stack->c_buffer() + FSW_HTTP2_FRAME_HEADER_SIZE;
    ssize_t payload_length = build_http_header(frame, req);

    if (payload_length <= 0)
    {
        return false;
    }

    frame->type = FSW_HTTP2_TYPE_HEADERS;
    frame->payload_length = payload_length;
    frame->stream_id = stream_id;
    frame->stream = stream;

    frame->flags = FSW_HTTP2_FLAG_END_HEADERS;
    /**
     * if don't need to send the body, can end the stream
     */
    if (!req->body_length)
    {
        frame->flags |= FSW_HTTP2_FLAG_END_STREAM;
    }

    build_frame_header(frame);

    streams.emplace(stream_id, stream);

    /**
     * send http headers
     */
    return send_frame(frame);
}

bool Client::send_http_body_frame(Frame *frame, Request *req)
{
    Stream *stream = frame->stream;
    frame->payload = FswG.buffer_stack->c_buffer() + FSW_HTTP2_FRAME_HEADER_SIZE;
    memcpy(frame->payload, req->body, req->body_length);

    frame->type = FSW_HTTP2_TYPE_DATA;
    frame->payload_length = req->body_length;
    frame->stream_id = frame->stream_id;
    frame->stream = stream;

    frame->flags = FSW_HTTP2_FLAG_END_STREAM;

    build_frame_header(frame);

    /**
     * send http body frame
     */
    return send_frame(frame);
}

int32_t Client::send_request(Request *req)
{
    Frame frame;

    send_http_header_frame(&frame, req);
    send_http_body_frame(&frame, req);

    stream_id += 2;

    return frame.stream->stream_id;
}

ssize_t Client::recv_frame(Frame *frame)
{
    Buffer *read_buf = sock->get_read_buf();
    char *buf = read_buf->c_buffer();

    sock->recv(buf, FSW_HTTP2_FRAME_HEADER_SIZE);
    ssize_t payload_length = get_payload_length(buf);
    sock->recv(buf + FSW_HTTP2_FRAME_HEADER_SIZE, payload_length);

    uint8_t type = buf[3];
    uint8_t flags = buf[4];
    uint32_t stream_id = ntohl((*(int *) (buf + 5))) & 0x7fffffff;

    buf += FSW_HTTP2_FRAME_HEADER_SIZE;

    frame->flags = flags;
    frame->payload = buf;
    frame->payload_length = payload_length;
    frame->stream_id = stream_id;
    frame->type = type;
    frame->stream = get_stream(stream_id);
    return FSW_HTTP2_FRAME_HEADER_SIZE + payload_length;
}

Response Client::recv_reponse()
{
    Frame frame;

    while (true)
    {
        recv_frame(&frame);
        enum fswReturn_code ret = parse_frame(&frame);
        if (ret == FSW_CONTINUE)
        {
            continue;
        }
        else if (ret == FSW_READY)
        {
            break;
        }
    }
    return frame.stream->response;
}
