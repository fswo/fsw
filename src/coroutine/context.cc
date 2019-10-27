#include "context.h"
#include "fsw.h"
#include "log.h"

using namespace fsw;

Context::Context(size_t stack_size, coroutine_func_t fn, void* private_data) :
        fn_(fn), stack_size_(stack_size), private_data_(private_data)
{
    swap_ctx_ = nullptr;

    try
    {
        stack_ = new char[stack_size_];
    }
    catch(const std::bad_alloc& e)
    {
        fswError("%s", e.what());
    }
    
    void* sp = (void*) ((char*) stack_ + stack_size_);
    ctx_ = make_fcontext(sp, stack_size_, (void (*)(intptr_t))&context_func);
}

Context::~Context()
{
    if (swap_ctx_)
    {
        delete[] stack_;
        stack_ = nullptr;
    }
}

bool Context::swap_in()
{
    jump_fcontext(&swap_ctx_, ctx_, (intptr_t) this, true);
    return true;
}

bool Context::swap_out()
{
    jump_fcontext(&ctx_, swap_ctx_, (intptr_t) this, true);
    return true;
}

void Context::context_func(void *arg)
{
    Context *_this = (Context *) arg;
    _this->fn_(_this->private_data_);
    _this->execute_defer_tasks();
    _this->end_ = true;
    _this->swap_out();
}

void Context::defer(coroutine_func_t _fn, void* _args)
{
    if (defer_tasks == nullptr)
    {
        defer_tasks = new std::stack<defer_task *>;
    }
    defer_task *task = new defer_task();
    task->fn = _fn;
    task->args = _args;
    defer_tasks->push(task);
}

void Context::execute_defer_tasks()
{
    if (defer_tasks) {
        defer_task *defer_task;
        while(!defer_tasks->empty())
        {
            defer_task = defer_tasks->top();
            defer_tasks->pop();
            fswDebug("here 1");
            defer_task->fn(defer_task->args);
            fswDebug("here 2");
            delete defer_task;
        }
        delete defer_tasks;
        defer_tasks = nullptr;
    }
}