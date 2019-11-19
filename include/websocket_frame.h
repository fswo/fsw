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

class Frame
{
public:
    struct FrameHeader header;
    char mask_key[MASK_LEN];
    uint16_t header_length;
    size_t payload_length;
    char *payload;

    void decode(Buffer *buffer);
    static void encode(Buffer *encode_buffer, Buffer *data);
    void debug();
    void fetch_payload(char *msg);

    inline void fetch_header(char *msg)
    {
        memcpy(&header, msg, HEADER_LEN);
    }
};
}
}

#endif	/* WEBSOCKET_FRAME_H */