#pragma once

#include "fsw.h"
#include "coroutine_socket.h"
#include "nghttp2.h"

using fsw::coroutine::Socket;

#define FSW_HTTP2_DATA_BUFFER_SIZE              8192
#define FSW_HTTP2_DEFAULT_HEADER_TABLE_SIZE     (1 << 12)
#define FSW_HTTP2_MAX_MAX_CONCURRENT_STREAMS    128
#define FSW_HTTP2_MAX_MAX_FRAME_SIZE            ((1u << 14))
#define FSW_HTTP2_MAX_WINDOW_SIZE               ((1u << 31) - 1)
#define FSW_HTTP2_DEFAULT_WINDOW_SIZE           65535
#define FSW_HTTP2_DEFAULT_MAX_HEADER_LIST_SIZE  (1 << 12)
#define FSW_HTTP2_MAX_MAX_HEADER_LIST_SIZE      UINT32_MAX

#define FSW_HTTP2_FRAME_HEADER_SIZE             9
#define FSW_HTTP2_FRAME_PING_PAYLOAD_SIZE       8

#define FSW_HTTP2_PRI_STRING  "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"

#define FSW_HTTP2_MAX_STATUS_LINE_NUM           8

namespace fsw { namespace coroutine { namespace http2 {

class Request;
class Response;
class Stream;

void set_frame_header(char *buffer, uint8_t type, uint32_t length, uint8_t flags, uint32_t stream_id);
ssize_t get_payload_length(char *buf);

enum stream_flag
{
    FSW_HTTP2_STREAM_NORMAL            = 0,
    FSW_HTTP2_STREAM_REQUEST_END       = 1 << 0,
    FSW_HTTP2_STREAM_PIPELINE_REQUEST  = 1 << 1,
    FSW_HTTP2_STREAM_PIPELINE_RESPONSE = 1 << 2,
};

enum frame_type
{
    FSW_HTTP2_TYPE_DATA = 0,
    FSW_HTTP2_TYPE_HEADERS = 1,
    FSW_HTTP2_TYPE_PRIORITY = 2,
    FSW_HTTP2_TYPE_RST_STREAM = 3,
    FSW_HTTP2_TYPE_SETTINGS = 4,
    FSW_HTTP2_TYPE_PUSH_PROMISE = 5,
    FSW_HTTP2_TYPE_PING = 6,
    FSW_HTTP2_TYPE_GOAWAY = 7,
    FSW_HTTP2_TYPE_WINDOW_UPDATE = 8,
    FSW_HTTP2_TYPE_CONTINUATION = 9,
};

enum frame_flag
{
    FSW_HTTP2_FLAG_NONE = 0x00,
    FSW_HTTP2_FLAG_ACK = 0x01,
    FSW_HTTP2_FLAG_END_STREAM = 0x01,
    FSW_HTTP2_FLAG_END_HEADERS = 0x04,
    FSW_HTTP2_FLAG_PADDED = 0x08,
    FSW_HTTP2_FLAG_PRIORITY = 0x20,
};

enum setting_id
{
    FSW_HTTP2_SETTING_HEADER_TABLE_SIZE       = 0x1,
    FSW_HTTP2_SETTINGS_ENABLE_PUSH            = 0x2,
    FSW_HTTP2_SETTINGS_MAX_CONCURRENT_STREAMS = 0x3,
    FSW_HTTP2_SETTINGS_INIT_WINDOW_SIZE       = 0x4,
    FSW_HTTP2_SETTINGS_MAX_FRAME_SIZE         = 0x5,
    FSW_HTTP2_SETTINGS_MAX_HEADER_LIST_SIZE   = 0x6,
};

typedef struct _settings
{
    uint32_t header_table_size;
    uint32_t window_size;
    uint32_t max_concurrent_streams;
    uint32_t max_frame_size;
    uint32_t max_header_list_size;
} settings;

/**
 * 9 byte frame header + payload
 +-----------------------------------------------+
 |                 Length (24)                   |
 +---------------+---------------+---------------+
 |   Type (8)    |   Flags (8)   |
 +-+-------------+---------------+-------------------------------+
 |R|                 Stream Identifier (31)                      |
 +=+=============================================================+
 |                   Frame Payload (0...)                      ...
 +---------------------------------------------------------------+
 */
class Frame
{
public:
    char *payload = nullptr;
    ssize_t payload_length;
    uint8_t type;
    uint8_t flags;
    uint32_t stream_id;
    Stream *stream;

    Frame(){};
    Frame(char *_payload, ssize_t _payload_length, uint8_t _type, uint8_t _flags, uint32_t _stream_id):
        payload(_payload), payload_length(_payload_length), type(_type), flags(_flags), stream_id(_stream_id){};
};

class Headers
{
public:
    Headers(size_t size) : size(size), index(0)
    {
        nvs = (nghttp2_nv *) calloc(size, sizeof(nghttp2_nv));
    }

    inline nghttp2_nv* get()
    {
        return nvs;
    }

    inline size_t len()
    {
        return index;
    }

    void reserve_one()
    {
        index++;
    }

    inline void add(
        size_t index,
        const char *name, size_t name_len,
        const char *value, size_t value_len,
        const uint8_t flags = NGHTTP2_NV_FLAG_NONE)
    {
        nghttp2_nv *nv = &nvs[index];
        nv->name = (uint8_t *) malloc(name_len);
        memcpy(nv->name, name, name_len);
        nv->namelen = name_len;
        nv->value = (uint8_t *) malloc(value_len);
        memcpy(nv->value, value, value_len);
        nv->valuelen = value_len;
        nv->flags = flags | NGHTTP2_NV_FLAG_NO_COPY_NAME | NGHTTP2_NV_FLAG_NO_COPY_VALUE;
    }

    inline void add(
        const char *name, size_t name_len,
        const char *value, size_t value_len,
        const uint8_t flags = NGHTTP2_NV_FLAG_NONE
    )
    {
        add(index++, name, name_len, value, value_len, flags);
    }

    ~Headers()
    {
        for (size_t i = 0; i < size; ++i)
        {
            if (nvs[i].name/* && nvs[i].value */)
            {
                free((void *) nvs[i].name);
                free((void *) nvs[i].value);
            }
        }
        free(nvs);
    }

private:
    nghttp2_nv *nvs;
    size_t size;
    size_t index;
};

class Request
{
public:
    std::string method;
    std::string path;
    uint32_t path_len;
    char *body = nullptr;
    size_t body_length;
    bool pipeline;

    /**
     * header_name is converted to lowercase
     */
    std::map<std::string, std::string> header;
};

class Response
{
public:
    uint32_t stream_id;
    std::string body;
    size_t body_length;
    int status_code;
    int err_code;
    std::map<std::string, std::string> header;
};

class Stream
{
public:
    uint32_t stream_id;
    uint8_t flags;
    Buffer *buffer = nullptr;
    Response response;

    // flow control
    uint32_t remote_window_size;
    uint32_t local_window_size;

    Stream(uint32_t stream_id, bool pipeline);
};

}
}
}