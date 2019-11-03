#include "fsw/fsw.h"
#include "fsw/coroutine_http.h"
#include "gtest/gtest.h"

using fsw::coroutine::http::Request;

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
