#include "fsw/fsw.h"
#include "fsw/buffer.h"
#include "fsw/coroutine_http.h"
#include "gtest/gtest.h"

using fsw::Buffer;
using fsw::coroutine::http::Response;

TEST(coroutine_http_request, set_header)
{
    Buffer header_name1(16);
    Buffer header_value1(16);
    header_name1.append("Connection");
    header_value1.append("Close");
    Buffer header_name2(16);
    Buffer header_value2(16);
    header_name2.append("Content-Type");
    header_value2.append("text/html");
    Response *response = new Response();
    response->set_header(&header_name1, &header_value1);
    response->set_header(&header_name2, &header_value2);
    response->set_header(&header_name2, &header_value2);
    ASSERT_EQ(response->header.size(), 3);
    ASSERT_NE(response->header[&header_name1], &header_value1);
    ASSERT_NE(response->header[&header_name2], &header_value2);
}
