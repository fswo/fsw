#include "fsw/coroutine_http.h"
#include "fsw/coroutine_http_server.h"
#include "fsw/coroutine.h"
#include "fsw/buffer.h"
#include "fsw/websocket_frame.h"
#include <functional>

using fsw::Coroutine;
using fsw::coroutine::run;

void task1(int p1)
{
    std::cout << p1 << std::endl;
}

void task2(int p1, std::string p2)
{
    std::cout << p1 << std::endl;
    std::cout << p2 << std::endl;
}

int main(int argc, char const *argv[])
{
    run([]()
    {
        Coroutine::create(std::bind(task1, 1));
        Coroutine::create(std::bind(task2, 1, "hello"));
    });

    return 0;
}