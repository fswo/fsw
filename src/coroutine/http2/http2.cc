#include "coroutine_http2.h"

using fsw::coroutine::http2::Stream;

Stream::Stream(uint32_t stream_id, bool pipeline)
{
    this->stream_id = stream_id;
    this->flags = pipeline ? FSW_HTTP2_STREAM_PIPELINE_REQUEST : FSW_HTTP2_STREAM_NORMAL;
    this->remote_window_size = FSW_HTTP2_DEFAULT_WINDOW_SIZE;
    this->local_window_size = FSW_HTTP2_DEFAULT_WINDOW_SIZE;
}