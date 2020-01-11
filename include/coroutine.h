#ifndef COROUTINE_H
#define COROUTINE_H

#include <unordered_map>
#include "context.h"
#include "fsw.h"
#include "log.h"
#include "event.h"

#define DEFAULT_C_STACK_SIZE          (2 *1024 * 1024)

using fsw::Event;
using fsw::FswG;

namespace fsw
{
class Coroutine
{
public:
    static std::unordered_map<long, Coroutine*> coroutines;

    static long create(std::function<void()> fn);
    static void yield();
    static void resume(long cid);
    static void resume(Coroutine *co);
    static int sleep(double seconds);
    static void defer(coroutine_func_t _fn, void* _args = nullptr);
    static Coroutine* get_current();

    inline long get_cid()
    {
        return cid;
    }

    static inline Coroutine* get_by_cid(long cid)
    {
        auto i = coroutines.find(cid);
        return i != coroutines.end() ? i->second : nullptr;
    }

protected:
    Coroutine *origin;
    static Coroutine* current;
    void *task = nullptr;
    static size_t stack_size;
    Context ctx;
    long cid;
    static long last_cid;

    long run();
    void _yield();
    void _resume();
    void* get_task();
    void set_task(void *_task);
    static void* get_current_task();
    void _defer(coroutine_func_t _fn, void* _args = nullptr);

    Coroutine(std::function<void ()> fn) :
            ctx(stack_size, fn)
    {
        cid = ++last_cid;
        coroutines[cid] = this;
    }
};

namespace coroutine
{
    inline void run(std::function<void()> fn)
    {
        FswG.event = new Event();
        Coroutine::create(fn);
        FswG.event->wait();
    }
}
}

#endif	/* COROUTINE_H */