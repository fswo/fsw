#include "process.h"

using fsw::Process;

Process::Process(std::function<void()> fn)
{
    handler = fn;
}

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
    handler();
    exit(0);
}

pid_t Process::wait()
{
    pid_t _pid;

    _pid = ::wait(NULL);
    return _pid;
}