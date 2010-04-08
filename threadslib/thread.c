/*
 * thread.c
 *
 *  Created on: April 8, 2010
 *      Author: Freifeld Royi
 */

#include "include/thread.h"
#include <stdlib.h>

/* machine context data structure */
typedef struct mctx_st
{
        ucontext_t uc;
} mctx_t;

/* save machine context */
#define mctx_savef(mctx) (void)getcontext(&(mctx)->uc)

/* restore machine context */
#define mctx_restore(mctx) (void)setcontext(&(mctx)->uc)

/* create machine context which can later be used to save & restore threads */
void mctx_create(mctx_t *mctx, void (*sf_addr)( ), void *sf_arg, void *sk_addr, size_t sk_size)
{
        /* fetch current context */
        getcontext(&(mctx->uc));
        /* adjust to new context */
        mctx->uc.uc_link = NULL;
        mctx->uc.uc_stack.ss_sp = sk_addr;
        mctx->uc.uc_stack.ss_size = sk_size;
        mctx->uc.uc_stack.ss_flags = 0;
        /* make new context */
        makecontext(&(mctx->uc), sf_addr, 1, sf_arg);
        return;
}

int create_thread(void(*sf_addr)(), void* sf_arg)
{

}
