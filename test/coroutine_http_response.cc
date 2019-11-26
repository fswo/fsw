#include "fsw/fsw.h"
#include "fsw/buffer.h"
#include "fsw/coroutine_http.h"
#include "gtest/gtest.h"

using fsw::Buffer;
using fsw::coroutine::http::Response;

TEST(coroutine_http_response, get_status_message)
{
    Response response;
    ASSERT_EQ(response.get_status_message(), "200 OK");
    response.set_status(404);
    ASSERT_EQ(response.get_status_message(), "404 Not Found");
}

TEST(coroutine_http_response, set_header_string)
{
    Response *response = new Response();
    response->set_header("Connection", "Close");
    response->set_header("Content-Type", "text/html");
    response->set_header("Content-Type", "text/html");
    ASSERT_EQ(response->header.size(), 2);
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
