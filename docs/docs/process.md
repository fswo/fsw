# Process

create process struct:

```cpp
Process(process_handler_t fn, bool enable_coroutine = false)
```

## read

read data from peer process:

```cpp
ssize_t fsw::Process::read(void *buf, size_t len)
```

## write

send data from peer process:

```cpp
ssize_t fsw::Process::write(const void *buf, size_t len)
```

## start

fork child process and call process handler:

```cpp
bool fsw::Process::start()
```

## wait

wait child process exit:

```cpp
pid_t fsw::Process::wait()
```

## process example

```cpp
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
```
