#ifndef CONTEXT_H
#define CONTEXT_H

#include "asm_context.h"

typedef fcontext_t coroutine_context_t;
typedef void (*coroutine_func_t)(void*);

struct defer_task
{
    coroutine_func_t fn;
    void *args;
};

namespace fsw
{
class Context
{
public:
    std::stack<defer_task *> *defer_tasks = nullptr;

    Context(size_t stack_size, coroutine_func_t fn, void* private_data);
    ~Context();
    bool swap_in();
    bool swap_out();
    static void context_func(void* arg); // coroutine entry function
    void defer(coroutine_func_t _fn, void* _args = nullptr);
    void execute_defer_tasks();
    
    inline bool is_end()
    {
        return end_;
    }

protected:
    coroutine_context_t ctx_;
    coroutine_context_t swap_ctx_;
    coroutine_func_t fn_;
    char* stack_;
    uint32_t stack_size_;
    void *private_data_;
    bool end_ = false;
};
}

#endif	/* CONTEXT_H */
