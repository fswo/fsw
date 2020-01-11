#include "coroutine.h"
#include "log.h"
#include "timer.h"

using fsw::Coroutine;
using fsw::Timer;

Coroutine* Coroutine::current = nullptr;
long Coroutine::last_cid = 0;
std::unordered_map<long, Coroutine*> Coroutine::coroutines;
size_t Coroutine::stack_size = DEFAULT_C_STACK_SIZE;

long Coroutine::run()
{
    long cid = this->cid;
    origin = current;
    current = this;
    ctx.swap_in();
    if (ctx.is_end())
    {
        fswTrace("coroutine[%ld] end", cid);
        current = origin;
        coroutines.erase(cid);
        delete this;
    }
    return cid;
}

void* Coroutine::get_current_task()
{
    return current ? current->get_task() : nullptr;
}

void* Coroutine::get_task()
{
    return task;
}

Coroutine* Coroutine::get_current()
{
    return current;
}

void Coroutine::set_task(void *_task)
{
    task = _task;
}

long Coroutine::create(std::function<void()> fn)
{
    return (new Coroutine(fn))->run();
}

void Coroutine::yield()
{
    Coroutine *co = Coroutine::get_current();
    co->_yield();
}

void Coroutine::_yield()
{
    assert(current == this);
    fswTrace("coroutine[%ld] yield", cid);
    current = origin;
    ctx.swap_out();
}

void Coroutine::resume(long cid)
{
    Coroutine *co = Coroutine::get_by_cid(cid);
    co->_resume();
}

void Coroutine::resume(Coroutine *co)
{
    co->_resume();
}

void Coroutine::_resume()
{
    assert(current != this);
    fswTrace("coroutine[%ld] resume", cid);
    origin = current;
    current = this;
    ctx.swap_in();
    if (ctx.is_end())
    {
        fswTrace("coroutine[%ld] end", cid);
        current = origin;
        coroutines.erase(cid);
        delete this;
    }
}

void Coroutine::defer(coroutine_func_t _fn, void* _args)
{
    Coroutine *co = Coroutine::get_current();
    co->_defer(_fn, _args);
}

void Coroutine::_defer(coroutine_func_t _fn, void* _args)
{
    ctx.defer(_fn, _args);
}

static void sleep_timeout(void *param)
{
    Coroutine::resume((Coroutine *) param);
}

int Coroutine::sleep(double seconds)
{
    fswTrace("coroutine[%ld] sleep", current->cid);

    FswG.event->timer_manager.add_timer(seconds * Timer::SECOND, sleep_timeout, (void*)current);
    Coroutine::yield();
    return 0;
}
