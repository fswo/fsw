#ifndef PROCESS_H_
#define PROCESS_H_

#include "fsw.h"

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

    bool start();
    pid_t wait();

    Process(process_handler_t fn, bool enable_coroutine = false):
        handler(fn), enable_coroutine(enable_coroutine) {}

    void name(std::string name)
    {
        prctl(PR_SET_NAME, name.c_str());
    }
};
}

#endif /* PROCESS_H_ */