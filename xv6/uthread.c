#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

struct context {
    uint edi, esi, ebx, ebp, eip;
};

#define MAX_THREADS 64
#define STACK_SIZE  4096

#define T_UNUSED   0
#define T_RUNNABLE 1
#define T_RUNNING  2
#define T_DONE     3

struct thread {
    int tid;
    int state;
    char *stack;
    struct context *ctx;
};

static struct thread threads[MAX_THREADS];
static struct thread *current;
static int next_tid = 1;

void thread_init(void) {
    memset(threads, 0, sizeof(threads));
    threads[0].tid   = next_tid++;
    threads[0].state = T_RUNNING;
    threads[0].stack = 0;
    current = &threads[0];
}

static void thread_entry(void (*fn)(void*), void *arg) {
    fn(arg);
    current->state = T_DONE;
    thread_yield();
}

tid_t thread_create(void (*fn)(void*), void *arg) {
    struct thread *t = 0;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == T_UNUSED) { t = &threads[i]; break; }
    }
    if (!t) return -1;

    t->stack = malloc(STACK_SIZE);
    if (!t->stack) return -1;

    uint *sp = (uint*)(t->stack + STACK_SIZE);
    *--sp = (uint)arg;
    *--sp = (uint)fn;
    *--sp = 0;
    *--sp = (uint)thread_entry;
    *--sp = 0; *--sp = 0; *--sp = 0; *--sp = 0;

    t->ctx   = (struct context*)sp;
    t->tid   = next_tid++;
    t->state = T_RUNNABLE;
    return t->tid;
}

void thread_yield(void) {
    int cur_idx = current - threads;
    int next_idx = -1;

    for (int i = 1; i <= MAX_THREADS; i++) {
        int idx = (cur_idx + i) % MAX_THREADS;
        if (threads[idx].state == T_RUNNABLE) { next_idx = idx; break; }
    }
    if (next_idx == -1) return;

    struct thread *prev = current;
    current = &threads[next_idx];
    if (prev->state == T_RUNNING)
        prev->state = T_RUNNABLE;
    current->state = T_RUNNING;
    uswtch(&prev->ctx, current->ctx);
}

int thread_join(tid_t tid) {
    struct thread *t = 0;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].tid == tid) { t = &threads[i]; break; }
    }
    if (!t) return -1;

    while (t->state != T_DONE)
        thread_yield();

    free(t->stack);
    t->state = T_UNUSED;
    return 0;
}
