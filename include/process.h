#pragma once

#include "fsw.h"
#include "unix_socket.h"

using fsw::UnixSocket;

namespace fsw
{
class Process
{
typedef void (*process_handler_t)(Process*);

private:
    process_handler_t handler;
    bool enable_coroutine;

public:
    pid_t pid;
    pid_t child_pid;
    UnixSocket *socket;

    ~Process();
    bool start();
    pid_t wait();
    ssize_t read(void *buf, size_t len);
    ssize_t write(const void *buf, size_t len);

    Process(process_handler_t fn, bool enable_coroutine = false):
        handler(fn), enable_coroutine(enable_coroutine)
    {
        socket = new UnixSocket();
    }

    void name(std::string name)
    {
        prctl(PR_SET_NAME, name.c_str());
    }
};
}
