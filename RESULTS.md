# Results

## Context-Switching Approach

Each thread has its own heap-allocated stack (4096 bytes) and a saved context
(registers edi, esi, ebx, ebp, eip) stored at the bottom of that stack.
Context switching is implemented via a naked inline assembly function (`swtch`)
that saves the current thread's callee-saved registers onto its stack, swaps
the stack pointer, and restores the next thread's registers. This mirrors the
kernel-level `swtch.S` in xv6.

Scheduling is cooperative and round-robin: threads call `thread_yield()` to
voluntarily give up the CPU. The scheduler scans the thread table for the next
`RUNNABLE` thread. Mutual exclusion is provided by a spinlock mutex that yields
while waiting, ensuring no thread holds the mutex while sleeping.

## Limitations

- Maximum threads: 64 (including the main thread)
- Stack size: 4096 bytes per thread (fixed at compile time)
- Cooperative only: a thread that never calls `thread_yield()` or `mutex_lock()`
  will starve all others
- No preemption, signals, or blocking I/O support
- Thread IDs are never reused within a process lifetime
