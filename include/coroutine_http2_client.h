#pragma once

#include "coroutine_http2.h"
#include "coroutine_socket.h"

using fsw::coroutine::Socket;
using fsw::coroutine::http2::Stream;

namespace fsw { namespace coroutine { namespace http2 {
class Client
{
public:
    Socket *sock = nullptr;

    settings local_settings = {};
    settings remote_settings = {};

    nghttp2_hd_inflater *inflater = nullptr;
    nghttp2_hd_deflater *deflater = nullptr;

    /**
     * the next send stream id
     */
    uint32_t stream_id = 0;
    /**
     * the last received stream id
     */
    uint32_t last_stream_id = 0;

    std::unordered_map<uint32_t, Stream*> streams;

    Client();
    void init_settings_local_settings();
    void init_settings_remote_settings();
    bool connect(std::string host, int port);
    int32_t send_request(Request *req);
    Response recv_reponse();
    ssize_t build_http_header(Request *req, char *buffer);
private:
    ssize_t recv_frame(Frame *frame);
    bool parse_header_stop(int inflate_flags, ssize_t inlen);
    void build_setting_frame(Frame *frame);
    void build_frame_header(Frame *frame);
    int parse_frame_header(Frame *frame);
    void parse_payload(Frame *frame);
    bool send(const char *buf, size_t len)
    {
        if (sock->send_all(buf, len) != (ssize_t )len)
        {
            return false;
        }
        return true;
    }

    Stream* get_stream(uint32_t stream_id)
    {
        auto i = streams.find(stream_id);
        if (i == streams.end())
        {
            return nullptr;
        }
        else
        {
            return i->second;
        }
    }

    void destroy_stream(Stream *stream)
    {
        if (stream->buffer)
        {
            delete stream->buffer;
            stream->buffer = nullptr;
        }
        delete stream;
    }

    bool delete_stream(uint32_t stream_id)
    {
        auto i = streams.find(stream_id);
        if (i == streams.end())
        {
            return false;
        }

        destroy_stream(i->second);
        streams.erase(i);

        return true;
    }

    fswReturn_code parse_frame(Frame *frame);
    int parse_setting_frame(Frame *frame);
};
}
}
}