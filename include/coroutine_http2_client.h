#pragma once

#include "coroutine_http2.h"

namespace fsw { namespace coroutine { namespace http2 {
class Client
{
public:
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
    int32_t send_request(Request *req);
    ssize_t build_header(Request *req, char *buffer);
};
}
}
}