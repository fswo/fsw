#include "process.h"
#include "coroutine.h"

using fsw::Process;
using fsw::Coroutine;

bool Process::start()
{
    pid_t _pid;

    _pid = fork();
    pid = getpid();
    child_pid = _pid;
    if (_pid > 0) // parent process
    {
        return true; // return child pid
    }

    // child process
    if (enable_coroutine)
    {
        Coroutine::create(std::bind(handler, this));
    }
    else
    {
        handler(this);
    }
    exit(0);
}

pid_t Process::wait()
{
    pid_t _pid;

    _pid = ::wait(NULL);
    return _pid;
}