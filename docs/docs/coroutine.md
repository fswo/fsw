# Coroutine

## create

create a coroutine and run it immediately:

```cpp
long Coroutine::create(coroutine_func_t fn, void * args)
```

```cpp
#include "fsw/coroutine.h"

using fsw::Coroutine;

int main(int argc, char const *argv[])
{
    fsw_event_init();

    Coroutine::create([](void *arg)
    {
        std::cout << "coroutine 1" << std::endl;
    });

    Coroutine::create([](void *arg)
    {
        std::cout << "coroutine 2" << std::endl;
    });

    Coroutine::create([](void *arg)
    {
        std::cout << "coroutine 3" << std::endl;
    });

    fsw_event_wait();

    return 0;
}
```

## get_current

get the currently running coroutine:

```cpp
static fsw::Coroutine *fsw::Coroutine::get_current()
```

```cpp
#include "fsw/coroutine.h"

using fsw::Coroutine;

int main(int argc, char const *argv[])
{
    fsw_event_init();

    Coroutine::create([](void *arg)
    {
        Coroutine *co = Coroutine::get_current();
    });

    fsw_event_wait();

    return 0;
}
```

## yield

swap out the current coroutine:

```cpp
static void fsw::Coroutine::yield()
```

```cpp
#include "fsw/coroutine.h"

using fsw::Coroutine;

int main(int argc, char const *argv[])
{
    fsw_event_init();

    long cid = Coroutine::create([](void *arg)
    {
        Coroutine *co = Coroutine::get_current();

        std::cout << 1 << std::endl;
        co->yield();
        std::cout << 3 << std::endl;
    });

    Coroutine::create([](void *arg)
    {
        long cid = (long)(uintptr_t)arg;

        std::cout << 2 << std::endl;
        Coroutine::resume(cid);
        std::cout << 4 << std::endl;
    }, (void*)(uintptr_t)cid);

    fsw_event_wait();

    return 0;
}
```

it will print:

```shell
1
2
3
4
```

## resume

swap in the specified coroutine:

```cpp
static void resume(long cid)
```

```cpp
#include "fsw/coroutine.h"

using fsw::Coroutine;

int main(int argc, char const *argv[])
{
    fsw_event_init();

    long cid = Coroutine::create([](void *arg)
    {
        Coroutine *co = Coroutine::get_current();
        std::cout << 1 << std::endl;
        co->yield();
        std::cout << 3 << std::endl;
    });

    Coroutine::create([](void *arg)
    {
        long cid = (long)(uintptr_t)arg;

        std::cout << 2 << std::endl;
        Coroutine::resume(cid);
        std::cout << 4 << std::endl;
    }, (void*)(uintptr_t)cid);

    fsw_event_wait();

    return 0;
}
```

it will print:

```shell
1
2
3
4
```

```cpp
static void fsw::Coroutine::resume(fsw::Coroutine *co)
```

```cpp
#include "fsw/coroutine.h"

using fsw::Coroutine;

int main(int argc, char const *argv[])
{
    fsw_event_init();

    long cid = Coroutine::create([](void *arg)
    {
        Coroutine *co = Coroutine::get_current();
        std::cout << 1 << std::endl;
        co->yield();
        std::cout << 3 << std::endl;
    });

    Coroutine::create([](void *arg)
    {
        long cid = (long)(uintptr_t)arg;
        Coroutine *co = Coroutine::get_by_cid(cid);

        std::cout << 2 << std::endl;
        Coroutine::resume(co);
        std::cout << 4 << std::endl;
    }, (void*)(uintptr_t)cid);

    fsw_event_wait();

    return 0;
}
```

it will print:

```shell
1
2
3
4
```

> notice: if you have already got a pointer to the coroutine,
> then please call directly resume(fsw::Coroutine *co) instead of resume(long cid),
> which will reduce the lookup of the map.

## defer

delayed execution of tasks:

```cpp
static void defer(coroutine_func_t _fn, void* _args = nullptr)
```

```cpp
#include "fsw/coroutine.h"

using fsw::Coroutine;

int main(int argc, char const *argv[])
{
    fsw_event_init();

    long cid = Coroutine::create([](void *arg)
    {
        Coroutine::defer([](void *arg)
        {
            std::cout << 1 << std::endl;
        });
        Coroutine::defer([](void *arg)
        {
            std::cout << 2 << std::endl;
        });
        Coroutine::defer([](void *arg)
        {
            std::cout << 3 << std::endl;
        });
    });
    fsw_event_wait();

    return 0;
}
```

it will print:

```shell
3
2
1
```

## sleep

let the current coroutine sleep:

```cpp
static int fsw::Coroutine::sleep(double seconds)
```

```cpp
#include "fsw/coroutine.h"
#include "fsw/timer.h"


using fsw::Coroutine;
using fsw::Timer;

int main(int argc, char const *argv[])
{
    fsw_event_init();

    long cid = Coroutine::create([](void *arg)
    {
        std::cout << 1 << std::endl;
        Coroutine::sleep(2 * Timer::MILLI_SECOND);
        std::cout << 2 << std::endl;
    });

    Coroutine::create([](void *arg)
    {
        std::cout << 3 << std::endl;
        Coroutine::sleep(1 * Timer::MILLI_SECOND);
        std::cout << 4 << std::endl;
    }, (void*)(uintptr_t)cid);

    fsw_event_wait();

    return 0;
}
```

it will print:

```shell
1
3
4
2
```
