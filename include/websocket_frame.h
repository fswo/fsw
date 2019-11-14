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
    unsigned char fin :1;
    unsigned char rsv1 :1;
    unsigned char rsv2 :1;
    unsigned char rsv3 :1;
    unsigned char opcode :4;
    unsigned char mask :1;
    unsigned char payload_len :7;
};

struct Frame
{
    struct FrameHeader header;
    char mask_key[MASK_LEN];
    char *payload;
};

void decode_frame(Buffer *buffer, struct Frame *frame);
void encode_frame(Buffer *encode_buffer, Buffer *data);

}
}

#endif	/* WEBSOCKET_FRAME_H */