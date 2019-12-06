# Channel

## Create

create a channel:

```cpp
fsw::coroutine::Channel::Channel(size_t _capacity = 1UL)
```

```cpp
#include "fsw/coroutine_channel.h"

using fsw::coroutine::Channel;
using fsw::coroutine::run;

int main(int argc, char const *argv[])
{
    run([](void *args)
    {
        Channel *chan1 = new Channel();
        Channel *chan2 = new Channel(2);
    });

    return 0;
}
```

## push

push the data into the channel:

```cpp
bool fsw::coroutine::Channel::push(void *data, double timeout = (-1.0))
```

```cpp
#include "fsw/coroutine.h"
#include "fsw/coroutine_channel.h"

using fsw::coroutine::Channel;
using fsw::Coroutine;
using fsw::coroutine::run;

int main(int argc, char const *argv[])
{
    run([](void *args)
    {
        Channel *chan = new Channel();

        Coroutine::create([](void *arg)
        {
            void *data;
            Channel *chan = (Channel *)arg;
            data = chan->pop();
            std::cout << *(std::string *)data << std::endl;
        }, (void *)chan);

        Coroutine::create([](void *arg)
        {
            bool ret;
            std::string data = "hello world";
            Channel *chan = (Channel *)arg;
            ret = chan->push(&data);
            std::cout << ret << std::endl;
        }, (void *)chan);

        delete chan;
    });
}
```

it will print:

```shell
hello world
1
```

## pop

pop the data from the channel:

```cpp
void *fsw::coroutine::Channel::pop(double timeout = (-1.0))
```
