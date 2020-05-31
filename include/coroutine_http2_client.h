#pragma once

#include "coroutine_http2.h"
#include "coroutine_socket.h"

using fsw::coroutine::Socket;

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
    ssize_t build_header(Request *req, char *buffer);
private:
    bool send(const char *buf, size_t len)
    {
        if (sock->send_all(buf, len) != (ssize_t )len)
        {
            return false;
        }
        return true;
    }

    bool parse_frame();
    bool parse_setting_frame(Frame *frame);
};
}
}
}