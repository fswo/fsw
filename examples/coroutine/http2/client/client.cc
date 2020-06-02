#include "fsw/coroutine_http2_client.h"
#include "fsw/coroutine.h"

using fsw::coroutine::http2::Client;
using fsw::coroutine::http2::Request;
using fsw::coroutine::http2::Response;
using fsw::coroutine::run;

int main(int argc, char const *argv[])
{
    run([]()
    {
        bool ret;
        Client h2c;
        Request req;
        Response rep;
        int32_t stream_id;

        ret = h2c.connect("127.0.0.1", 80);
        if (!ret)
        {
            fswError("%s", h2c.sock->get_err_msg());
        }

        req.method = "GET";
        req.path = "/";

        req.header["host"] = "127.0.0.1";
        req.header["user-agent"] = "Chrome/49.0.2587.3";
        req.header["content-type"] = "text/html";

        req.body = (char *) "http2 client";
        req.body_length = strlen(req.body);

        stream_id = h2c.send_request(&req);
        std::cout << stream_id << std::endl;
        rep = h2c.recv_reponse();
        std::cout << rep.body << std::endl;

        return 0;
    });
}
