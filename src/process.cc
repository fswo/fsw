#include "process.h"
#include "coroutine.h"

using fsw::Process;
using fsw::Coroutine;
using fsw::UnixSocket;

Process::~Process()
{
    delete socket;
}

bool Process::start()
{
    pid_t _pid;

    _pid = fork();
    pid = getpid();
    child_pid = _pid;
    if (_pid > 0) // parent process
    {
        socket->current_fd = socket->get_socket_fd(UnixSocket::type::MASTER);
        return true;
    }

    // child process
    socket->current_fd = socket->get_socket_fd(UnixSocket::type::WORKER);
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

ssize_t Process::read(void *buf, size_t len)
{
    return socket->recv(buf, len);
}

ssize_t Process::write(const void *buf, size_t len)
{
    return socket->send(buf, len);
}