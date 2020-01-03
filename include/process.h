#ifndef PROCESS_H_
#define PROCESS_H_

#include "fsw.h"

namespace fsw
{
class Process
{
private:
    std::function<void()> handler;
public:
    pid_t pid;
    pid_t child_pid;

    Process(std::function<void()> fn);
    bool start();
    pid_t wait();
};
}

#endif /* PROCESS_H_ */