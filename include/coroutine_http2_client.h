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

    Client();
    void init_settings_local_settings();
    int32_t send_request(Request *req);
    ssize_t build_header(Request *req, char *buffer);
};
}
}
}