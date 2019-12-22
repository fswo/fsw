#include "fsw/fsw.h"
#include "fsw/coroutine_http.h"
#include "gtest/gtest.h"

using fsw::coroutine::http::Request;

TEST(coroutine_http_request, has_header)
{
    Request *request = new Request();
    ASSERT_FALSE(request->has_header("fsw"));
    request->header["fsw"] = "hello";
    ASSERT_TRUE(request->has_header("fsw"));
}

TEST(coroutine_http_request, get_header)
{
    Request *request = new Request();
    std::string value = request->get_header("fsw");
    ASSERT_EQ(value, "");
    request->header["fsw"] = "hello";
    value = request->get_header("fsw");
    ASSERT_EQ(value, "hello");
}

TEST(coroutine_http_request, header_contain_value)
{
    Request *request = new Request();
    ASSERT_FALSE(request->header_contain_value("fsw", "hello"));
    request->header["fsw"] = "hello";
    ASSERT_TRUE(request->header_contain_value("fsw", "hello"));
}

TEST(coroutine_http_request, has_sec_websocket_key)
{
    Request *request = new Request();
    ASSERT_FALSE(request->has_sec_websocket_key());
    request->header["sec-websocket-key"] = "xxxx";
    ASSERT_TRUE(request->has_sec_websocket_key());
}

TEST(coroutine_http_request, clear)
{
    char path[] = "/index";
    char body[] = "hello world";

    Request *request = new Request();
    request->path = new char[7];
    memcpy(request->path, path, 7);
    request->path_len = 7;

    request->header["fsw"] = "hello";

    request->body = new char[12];
    memcpy(request->body, body, 12);
    request->body_length = 12;

    request->clear();

    ASSERT_EQ(request->path, nullptr);
    ASSERT_EQ(request->path_len, 0);

    ASSERT_EQ(request->header.size(), 0);

    ASSERT_EQ(request->body, nullptr);
    ASSERT_EQ(request->body_length, 0);
}

TEST(coroutine_http_request, clear_header)
{
    Request *request = new Request();
    request->header["fsw"] = "hello";
    request->clear_header();
    ASSERT_EQ(request->header.size(), 0);
}

TEST(coroutine_http_request, clear_path)
{
    char path[] = "/index";
    Request *request = new Request();
    request->path = new char[7];
    memcpy(request->path, path, 7);
    request->path_len = 7;
    ASSERT_NE(request->path, path);
    ASSERT_EQ(memcmp(request->path, path, 7), 0);
    ASSERT_EQ(request->path_len, 7);
    request->clear_path();
    ASSERT_EQ(request->path, nullptr);
    ASSERT_EQ(request->path_len, 0);
}

TEST(coroutine_http_request, clear_body)
{
    char body[] = "hello world";
    Request *request = new Request();
    request->body = new char[12];
    memcpy(request->body, body, 12);
    request->body_length = 12;
    ASSERT_NE(request->body, body);
    ASSERT_EQ(memcmp(request->body, body, 12), 0);
    ASSERT_EQ(request->body_length, 12);
    request->clear_body();
    ASSERT_EQ(request->body, nullptr);
    ASSERT_EQ(request->body_length, 0);
}
