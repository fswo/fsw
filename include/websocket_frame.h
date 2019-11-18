#ifndef WEBSOCKET_FRAME_H
#define WEBSOCKET_FRAME_H

#include "fsw.h"
#include "buffer.h"

using fsw::Buffer;

namespace fsw { namespace websocket {

static unsigned int const MASK_LEN = 4;
static unsigned int const HEADER_LEN = 2;

struct FrameHeader
{
    unsigned char opcode :4;
    unsigned char rsv3 :1;
    unsigned char rsv2 :1;
    unsigned char rsv1 :1;
    unsigned char fin :1;
    /**
     * if length < 126, length is payload's true length,
     * else length >= 126, length is not payload's true length.
     */
    unsigned char length :7;
    unsigned char mask :1;
};

struct Frame
{
    struct FrameHeader header;
    char mask_key[MASK_LEN];
    uint16_t header_length; // header's true length
    size_t payload_length; // payload's true length
    char *payload;
};

void decode_frame(Buffer *buffer, struct Frame *frame);
void encode_frame(Buffer *encode_buffer, Buffer *data);
void debug_frame(struct Frame *frame);

}
}

#endif	/* WEBSOCKET_FRAME_H */