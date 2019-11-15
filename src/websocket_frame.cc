#include "buffer.h"
#include "websocket_frame.h"
#include "coroutine_socket.h"

using fsw::Buffer;
using fsw::coroutine::Socket;

/*  The following is websocket data frame:
 +-+-+-+-+-------+-+-------------+-------------------------------+
 0                   1                   2                   3   |
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 |
 +-+-+-+-+-------+-+-------------+-------------------------------+
 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 | |1|2|3|       |K|             |                               |
 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 |     Extended payload length continued, if payload len == 127  |
 + - - - - - - - - - - - - - - - +-------------------------------+
 |     payload length (continue) |Masking-key, if MASK set to 1  |
 +-------------------------------+-------------------------------+
 | Masking-key (continued)       |          Payload Data         |
 +-------------------------------- - - - - - - - - - - - - - - - +
 :                     Payload Data continued ...                :
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+
 */

namespace fsw { namespace websocket {

static inline uint64_t ntoh64(uint64_t net)
{
    uint64_t ret = 0;
    uint32_t high, low;

    low = net & 0xFFFFFFFF;
    high = net >> 32;
    low = ntohl(low);
    high = ntohl(high);

    ret = low;
    ret <<= 32;
    ret |= high;
    return ret;
}

static inline void mask(char *data, size_t len, const char *mask_key)
{
    size_t n = len / 8;
    uint64_t mask_key64 = ((uint64_t) (*((uint32_t *) mask_key)) << 32) | *((uint32_t *) mask_key);
    size_t i;

    for (i = 0; i < n; i++)
    {
        ((uint64_t *) data)[i] ^= mask_key64;
    }

    for (i = n * 8; i < len; i++)
    {
        data[i] ^= mask_key[i % MASK_LEN];
    }
}

static inline void fetch_fin(struct Frame *frame, char *msg)
{
    frame->header.fin = (unsigned char)msg[0] & 0x80;
    return;
}

static inline void fetch_mask(struct Frame *frame, char *msg)
{
    frame->header.mask = (unsigned char)msg[1] & 0x80;
    return;
}

static inline void fetch_opcode(struct Frame *frame, char *msg)
{
    frame->header.opcode = (unsigned char) msg[0] & 0x0F;
    return;
}

static inline void fetch_payload(struct Frame *frame, char *msg)
{
    uint8_t header_length = HEADER_LEN;
    frame->header.length = msg[1] & 0x7f;
    size_t payload_length = frame->header.length;
    char *buf = msg + HEADER_LEN;

    if (frame->header.length == 126)
    {  
        payload_length = ntohs(*((uint16_t *) buf));
        header_length += 2;
    }  
    else if (frame->header.length == 127)
    {  

        payload_length = ntoh64(*((uint64_t *) buf));
        header_length += 8;
    }

    if (frame->header.mask)
    {
        memcpy(frame->mask_key, msg + header_length, MASK_LEN);
        header_length += MASK_LEN;
        if (payload_length > 0)
        {
            mask(msg + header_length, payload_length, frame->mask_key);
        }
    }
    frame->header_length = header_length;
    frame->payload = msg + header_length;
    frame->payload_length = payload_length;
    return;
}

void decode_frame(Buffer *buffer, struct Frame *frame)
{
    fetch_fin(frame, buffer->c_buffer());
    fetch_opcode(frame, buffer->c_buffer());
    fetch_mask(frame, buffer->c_buffer());
    fetch_payload(frame, buffer->c_buffer());
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
    header->payload_length = data->length();

    encode_buffer->append(frame_header, sizeof(frame_header));
    encode_buffer->append(data);
}

}
}
