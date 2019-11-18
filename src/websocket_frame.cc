#include "buffer.h"
#include "websocket_frame.h"
#include "coroutine_socket.h"
#include "log.h"

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

static inline uint64_t hton64(uint64_t host)
{
    uint64_t ret = 0;
    uint32_t high, low;

    low = host & 0xFFFFFFFF;
    high = (host >> 32) & 0xFFFFFFFF;
    low = htonl(low);
    high = htonl(high);

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

static inline void fetch_header(struct Frame *frame, char *msg)
{
    memcpy(frame, msg, HEADER_LEN);
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
    fetch_header(frame, buffer->c_buffer());
    fetch_payload(frame, buffer->c_buffer());
}

void encode_frame(Buffer *encode_buffer, Buffer *data)
{
    int pos = 0;
    char frame_header[16];
    struct FrameHeader *header = (struct FrameHeader *)frame_header;

    header->fin = 1;
    header->opcode = 1;
    header->rsv1 = 0;
    header->rsv2 = 0;
    header->rsv3 = 0;
    header->mask = 0;
    pos = 2;

    if (data->length() < 126)
    {
        header->length = data->length();
    }
    else if (data->length() < 65536)
    {
        header->length = 126;
        uint16_t *length_ptr = (uint16_t *) (frame_header + pos);
        *length_ptr = htons(data->length());
        pos += sizeof(*length_ptr);
    }
    else
    {
        header->length = 127;
        uint64_t *length_ptr = (uint64_t *)(frame_header + pos);
        *length_ptr = hton64(data->length());
        pos += sizeof(*length_ptr);
    }
    encode_buffer->append(frame_header, pos);

    if (header->mask)
    {
        encode_buffer->append("258E");
        if (data->length() > 0)
        {
            size_t offset = encode_buffer->length();
            encode_buffer->append(data);
            mask(encode_buffer->c_buffer() + offset, data->length(), "258E");
        }
    }
    else
    {
        if (data->length() > 0)
        {
            encode_buffer->append(data);
        }
    }
}

void debug_frame(struct Frame *frame)
{
    fswDebug("header->header.fin: %u", frame->header.fin);
    fswDebug("header->header.rsv1: %u", frame->header.rsv1);
    fswDebug("header->header.rsv2: %u", frame->header.rsv2);
    fswDebug("header->header.rsv3: %u", frame->header.rsv3);
    fswDebug("header->header.opcode: %u", frame->header.opcode);
    fswDebug("header->header.mask: %u", frame->header.mask);
    fswDebug("header->header.length: %u", frame->header.length);
    fswDebug("header->header.header_length: %u", frame->header_length);
    fswDebug("header->header.payload_length: %zu", frame->payload_length);
}

}
}
