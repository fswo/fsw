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

namespace fsw { namespace coroutine { namespace http2 {

class Request;
class Response;

typedef struct _settings
{
    uint32_t header_table_size;
    uint32_t window_size;
    uint32_t max_concurrent_streams;
    uint32_t max_frame_size;
    uint32_t max_header_list_size;
} settings;

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
    char *path = nullptr;
    uint32_t path_len;
    char *body = nullptr;
    size_t body_length;
    bool pipeline;

    /**
     * header_name is converted to lowercase
     */
    std::map<std::string, std::string> header;
};

}
}
}