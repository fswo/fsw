#include "coroutine_http2.h"

using fsw::coroutine::http2::Stream;

Stream::Stream(uint32_t stream_id, bool pipeline)
{
    this->stream_id = stream_id;
    this->flags = pipeline ? FSW_HTTP2_STREAM_PIPELINE_REQUEST : FSW_HTTP2_STREAM_NORMAL;
    this->remote_window_size = FSW_HTTP2_DEFAULT_WINDOW_SIZE;
    this->local_window_size = FSW_HTTP2_DEFAULT_WINDOW_SIZE;
}

namespace fsw { namespace coroutine { namespace http2 {

void set_frame_header(char *buffer, uint8_t type, uint32_t length, uint8_t flags, uint32_t stream_id)
{
    buffer[0] = length >> 16;
    buffer[1] = length >> 8;
    buffer[2] = length;
    buffer[3] = type;
    buffer[4] = flags;
    *(uint32_t *) (buffer + 5) = htonl(stream_id);
}

}
}
}
