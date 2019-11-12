#include "websocket_frame.h"

namespace fsw { namespace websocket {

void decode_frame(Buffer *buffer, struct Frame *frame)
{
    memcpy(frame, buffer->c_buffer(), HEADER_LEN);

    /**
     * payload_len: 0~125,
     * 
     * TODO:
     * because, later, the case where payload_len is greater than 125 will be supported, 
     * so here we define additional variables to save payload_len and header_len.
     */
    size_t payload_len = frame->header.payload_len;
    uint8_t header_len = HEADER_LEN;
    char *payload = buffer->c_buffer() + HEADER_LEN;
}

}
}
