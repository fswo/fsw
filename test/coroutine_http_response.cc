#include "fsw/fsw.h"
#include "fsw/buffer.h"
#include "fsw/coroutine_http.h"
#include "gtest/gtest.h"

using fsw::Buffer;
using fsw::coroutine::http::Response;

TEST(coroutine_http_response, set_header)
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

TEST(coroutine_http_response, update_header)
{
    Buffer header_name1(16);
    Buffer header_value1(16);
    header_name1.append("Connection");
    header_value1.append("Close");
    Buffer header_value2(16);
    header_value2.append("Keep-Alive");
    Response *response = new Response();
    response->set_header(&header_name1, &header_value1);
    response->update_header(&header_name1, &header_value2);
    ASSERT_EQ(response->header.size(), 1);
}

TEST(coroutine_http_response, set_version)
{
    Response response;

    response.set_version(100);
    ASSERT_EQ(response._version, 100);
}

TEST(coroutine_http_response, get_real_version)
{
    Response response;
    std::string real_version;

    response.set_version(100);
    real_version = response.get_real_version();
    ASSERT_EQ(real_version, "1.0");

    response.set_version(400);
    real_version = response.get_real_version();
    ASSERT_EQ(real_version, "Unknown version: 400");
}
