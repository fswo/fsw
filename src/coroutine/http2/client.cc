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

ssize_t Client::build_header(Request *req, char *buffer)
{
    Headers headers(8 + req->header.size());
    headers.add(FSW_STRL(":method"), FSW_STRL("GET"));
    headers.add(FSW_STRL(":path"), FSW_STRL("/"));
    headers.add(FSW_STRL(":scheme"), FSW_STRL("http"));

    headers.reserve_one();

    size_t buflen = nghttp2_hd_deflate_bound(deflater, headers.get(), headers.len());
    ssize_t rv = nghttp2_hd_deflate_hd(deflater, (uint8_t *) buffer, buflen, headers.get(), headers.len());
    if (rv < 0)
    {
        fswError("Error has occurred: (errno %zu) %s", rv, "nghttp2_hd_deflate_hd() failed");
        return FSW_ERR;
    }
    return rv;
}

fswReturn_code Client::parse_frame()
{
    char *buf = sock->get_read_buf()->c_buffer();
    uint8_t type = buf[3];
    uint8_t flags = buf[4];
    uint32_t stream_id = ntohl((*(int *) (buf + 5))) & 0x7fffffff;
    ssize_t payload_length = get_payload_length(buf);

    buf += FSW_HTTP2_FRAME_HEADER_SIZE;

    Frame frame(buf, payload_length, type, flags, stream_id);

    if (stream_id > last_stream_id)
    {
        last_stream_id = stream_id;
    }

    switch (type)
    {
    case FSW_HTTP2_TYPE_SETTINGS:
        parse_setting_frame(&frame);
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

    if (type == FSW_HTTP2_TYPE_HEADERS)
    {
        parse_header(&frame);
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

int Client::parse_header(Frame *frame)
{
    Stream *stream = get_stream(frame->stream_id);
    char *in = frame->payload;
    ssize_t inlen = frame->payload_length;
    Response response = stream->response;

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

bool Client::parse_setting_frame(Frame *frame)
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
                    return false;
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
        return false;
    }
    return true;
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

    if (!send(FSW_STRL(FSW_HTTP2_PRI_STRING)))
    {
        return false;
    }
    return true;
}

int32_t Client::send_request(Request *req)
{
    /**
     * send headers
     */
    char *buffer = FswG.buffer_stack->c_buffer();
    ssize_t bytes = build_header(req, buffer + FSW_HTTP2_FRAME_HEADER_SIZE);

    if (bytes <= 0)
    {
        return FSW_ERR;
    }

    Stream *stream = new Stream(stream_id, false);
    streams.emplace(stream_id, stream);

    if (stream->flags & FSW_HTTP2_STREAM_PIPELINE_REQUEST)
    {
        set_frame_header(buffer, FSW_HTTP2_TYPE_HEADERS, bytes, FSW_HTTP2_FLAG_END_HEADERS, stream->stream_id);
    }
    else
    {
        set_frame_header(buffer, FSW_HTTP2_TYPE_HEADERS, bytes, FSW_HTTP2_FLAG_END_STREAM | FSW_HTTP2_FLAG_END_HEADERS, stream->stream_id);
    }

    if (!send(buffer, FSW_HTTP2_FRAME_HEADER_SIZE + bytes))
    {
        return FSW_OK;
    }

    stream_id += 2;

    return stream->stream_id;
}

ssize_t Client::recv_frame()
{
    Buffer *read_buf = sock->get_read_buf();
    char *buf = read_buf->c_buffer();

    sock->recv(buf, FSW_HTTP2_FRAME_HEADER_SIZE);
    ssize_t payload_length = get_payload_length(buf);
    sock->recv(buf + FSW_HTTP2_FRAME_HEADER_SIZE, payload_length);
}

Response Client::recv_reponse()
{
    while (true)
    {
        recv_frame();
        enum fswReturn_code ret = parse_frame();
        if (ret == FSW_CONTINUE)
        {
            continue;
        }
    }
}
