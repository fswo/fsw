#include "buffer.h"
#include "websocket_frame.h"
#include "coroutine_socket.h"

using fsw::Buffer;
using fsw::coroutine::Socket;

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

void encode_frame(Buffer *encode_buffer, Buffer *data)
{

    char frame_header[2];
    struct Frame *header = (struct Frame *)frame_header;

    header->header.fin = 0;
    header->header.rsv1 = 0;
    header->header.rsv2 = 0;
    header->header.rsv3 = 0;
    header->header.opcode = 1;
    header->header.mask = 0;
    header->header.payload_len = data->length();

    encode_buffer->append(frame_header, sizeof(frame_header));
    encode_buffer->append(data);
}

}
}
