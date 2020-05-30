#include "fsw/coroutine_http2_client.h"
#include "fsw/coroutine.h"

using fsw::coroutine::http2::Client;
using fsw::coroutine::http2::Request;
using fsw::coroutine::run;


int main(int argc, char const *argv[])
{
    run([]()
    {
        bool ret;
        Client h2c;
        Request req;

        ret = h2c.connect("127.0.0.1", 80);
        if (!ret)
        {
            fswError("%s", h2c.sock->get_err_msg());
        }
        h2c.send_request(&req);
        return 0;
    });
}
