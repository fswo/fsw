#include "fsw/process.h"

using fsw::Process;

int main(int argc, char const *argv[])
{
    Process proc([](Process *proc)
    {
        char buf[6];
        proc->read(buf, 5);
        buf[6] = 0;
        std::cout << buf << std::endl;
    });

    proc.start();
    proc.write("hello", 5);
    proc.wait();
    return 0;
}
