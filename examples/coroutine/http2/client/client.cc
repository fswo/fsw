#include "fsw/coroutine_http2_client.h"
#include "fsw/coroutine.h"

using fsw::coroutine::http2::Client;
using fsw::coroutine::http2::Request;
using fsw::coroutine::run;


int main(int argc, char const *argv[])
{
    run([]()
    {
        Client h2c;
        Request req;
        h2c.send_request(&req);
        return 0;
    });
}
