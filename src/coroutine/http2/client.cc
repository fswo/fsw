#include "coroutine_http2_client.h"
#include "event.h"
#include "log.h"
#include "socket.h"

using fsw::coroutine::http2::Client;
using fsw::coroutine::http2::Headers;
using fsw::FswG;

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
    local_settings.header_table_size = FSW_HTTP2_DEFAULT_HEADER_TABLE_SIZE;
    local_settings.window_size = FSW_HTTP2_DEFAULT_WINDOW_SIZE;
    local_settings.max_concurrent_streams = FSW_HTTP2_MAX_MAX_CONCURRENT_STREAMS;
    local_settings.max_frame_size = FSW_HTTP2_MAX_MAX_FRAME_SIZE;
    local_settings.max_header_list_size = FSW_HTTP2_DEFAULT_MAX_HEADER_LIST_SIZE;
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
