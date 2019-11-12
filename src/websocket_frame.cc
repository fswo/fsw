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

size_t recv(Ctx *ctx, struct Frame *frame)
{
    ssize_t recved;
    Socket *conn = ctx->conn;
    recved = conn->recv(conn->get_read_buf()->c_buffer(), READ_BUF_MAX_SIZE);
    Buffer buf(recved);
    buf.append(conn->get_read_buf()->c_buffer(), recved);

    decode_frame(&buf, frame);
}

}
}
