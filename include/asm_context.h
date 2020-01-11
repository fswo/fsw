#pragma once

#include "fsw.h"

typedef void* fcontext_t;

extern intptr_t jump_fcontext(fcontext_t *ofc, fcontext_t nfc, intptr_t vp, bool preserve_fpu = false) asm("jump_fcontext");
extern fcontext_t make_fcontext(void *sp, size_t size, void (*fn)(intptr_t)) asm("make_fcontext");
