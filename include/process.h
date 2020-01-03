#ifndef PROCESS_H_
#define PROCESS_H_

#include "fsw.h"

namespace fsw
{
class Process
{
private:
    std::function<void()> handler;
    bool enable_coroutine;

public:
    pid_t pid;
    pid_t child_pid;

    bool start();
    pid_t wait();

    Process(std::function<void()> fn, bool enable_coroutine = false):
        handler(fn), enable_coroutine(enable_coroutine) {}
};
}

#endif /* PROCESS_H_ */