#ifndef COROUTINE_H
#define COROUTINE_H

#include <unordered_map>
#include "context.h"
#include "fsw.h"
#include "log.h"

#define DEFAULT_C_STACK_SIZE          (2 *1024 * 1024)

namespace fsw
{
class Coroutine
{
public:
    static std::unordered_map<long, Coroutine*> coroutines;

    long run();
    static void* get_current_task();
    static long create(coroutine_func_t fn, void* args = nullptr);
    void* get_task();
    static Coroutine* get_current();
    void set_task(void *_task);
    void yield();
    void resume();
    void defer(coroutine_func_t _fn, void* _args = nullptr);
    static int sleep(double seconds);

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

    Coroutine(coroutine_func_t fn, void *private_data) :
            ctx(stack_size, fn, private_data)
    {
        cid = ++last_cid;
        coroutines[cid] = this;
    }
};
}

#endif	/* COROUTINE_H */