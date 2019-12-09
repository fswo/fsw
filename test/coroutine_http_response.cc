#include "fsw/fsw.h"
#include "fsw/buffer.h"
#include "fsw/coroutine_http.h"
#include "gtest/gtest.h"

using fsw::Buffer;
using fsw::coroutine::http::Ctx;
using fsw::coroutine::http::Response;
using fsw::coroutine::http::Request;

TEST(coroutine_http_response, get_status_message)
{
    Response response;
    ASSERT_EQ(response.get_status_message(), "200 OK");
    response.set_status(404);
    ASSERT_EQ(response.get_status_message(), "404 Not Found");
}

TEST(coroutine_http_response, build_http_status_line)
{
    Response response;
    Socket *sock = new Socket(1);
    Ctx *ctx = new Ctx(sock);
    response.ctx = ctx;
    response.set_status(404);
    response.build_http_status_line();
    Buffer *buf = response.get_write_buf();
    ASSERT_TRUE(buf->equal("HTTP/1.1 404 Not Found\r\n"));
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

TEST(coroutine_http_response, clear_header)
{
    Response response;
    Socket *sock = new Socket(1);
    Ctx *ctx = new Ctx(sock);
    response.ctx = ctx;
    response.header["fsw"] = "hello";
    response.clear_header();
    ASSERT_EQ(response.header.size(), 0);
}

TEST(coroutine_http_response, check_websocket_upgrade)
{
    Request request;
    Response response;
    Socket *sock = new Socket(1);
    Ctx *ctx = new Ctx(sock);

    ctx->request = &request;
    ctx->response = &response;
    response.ctx = ctx;

    request.method = "GET";
    request.header["connection"] = "upgrade";
    request.header["upgrade"] = "websocket";
    request.header["sec-websocket-version"] = "13";
    request.header["sec-websocket-key"] = "xxxx";

    std::string sec_websocket_key = ctx->request->get_header("sec-websocket-key");
    ASSERT_TRUE(ctx->response->check_websocket_upgrade(sec_websocket_key));
}