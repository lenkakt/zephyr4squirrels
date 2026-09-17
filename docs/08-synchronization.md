<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [7. Threads, extended](07-threads-extended.md)

# 8. Synchronization: locks

[Steps 6–7](06-threads-basics.md) had two threads that never touched each
other's data — each one only ever read its own name and period. Real
programs are rarely that tidy: threads often need to share a variable, a
buffer, or a piece of state. This step is about what goes wrong when they
do, and the simplest tool for fixing it. See the official
[Mutexes](https://docs.zephyrproject.org/latest/kernel/services/synchronization/mutexes.html)
documentation for the complete reference.

## The bug

Look at [apps/sync/src/main.c](../apps/sync/src/main.c) — two threads,
`racer1` and `racer2`, both increment the same `shared_counter`, five
times each:

```c
static int shared_counter;

static void increment_unsafe(void)
{
	int temp = shared_counter;

	k_yield();
	shared_counter = temp + 1;
}
```

Build and run it:

```sh
# repo root
west build -b m5stack_fire/esp32/procpu -d build apps/sync
./scripts/run-qemu.sh build 16
```

```
*** Booting Zephyr OS build v4.4.2 ***
shared_counter = 5 (expected 10)
```

Ten increments happened. Five got lost. Run it again — same result, every
time, not a one-off fluke.

## Why

`shared_counter++` looks like one step. It isn't: it's a **read**, then
(separately) a **write**. `increment_unsafe()` deliberately splits those
two apart with a `k_yield()` in between, which hands control to the other
thread immediately — right in the middle of the operation. That's the
whole bug: the scheduler is always free to switch threads between any two
instructions, and normally that's exactly what you want (it's why
[Step 6](06-threads-basics.md) worked at all). It just doesn't stop to ask
whether you were in the middle of something with shared data.

Walk through what actually happens: `racer1` reads `shared_counter` (say,
0), then yields before writing anything back. `racer2` runs, reads the
*same* 0 (racer1 hasn't written yet), and yields too. Now racer1 resumes
and writes `0 + 1 = 1`. Then racer2 resumes — but it already has its own
`temp = 0` from before, so it writes `0 + 1 = 1` again, silently
overwriting racer1's update. Two increments, one net change. That's
precisely why the result lands at exactly half of what was expected,
every single run: the `k_yield()` forces this exact interleaving
deterministically, instead of leaving it to chance the way a real,
untimed race condition would. The underlying bug is identical either
way — this just makes it reproducible on demand instead of "usually fine,
occasionally very wrong at 3am in production."

## The fix: a mutex

A **mutex** ("mutual exclusion") is a lock: only one thread can hold it at
a time. A second thread trying to lock an already-held mutex **blocks** —
it waits — instead of barging in.

```c
K_MUTEX_DEFINE(counter_lock);

static void increment_safe(void)
{
	k_mutex_lock(&counter_lock, K_FOREVER);

	int temp = shared_counter;

	k_yield();
	shared_counter = temp + 1;

	k_mutex_unlock(&counter_lock);
}
```

`K_MUTEX_DEFINE` is the same declare-a-kernel-object-at-file-scope pattern
you've now seen for threads (`K_THREAD_DEFINE`) and shell commands
(`SHELL_CMD_REGISTER`). `k_mutex_lock(&counter_lock, K_FOREVER)` waits as
long as it takes to acquire the lock; `k_mutex_unlock()` releases it.
Everything between those two calls is the **critical section** — the part
that must never run on two threads at once.

Edit `apps/sync/src/main.c` yourself: add the `K_MUTEX_DEFINE` line, wrap
`increment_unsafe`'s body in `lock`/`unlock` (rename it `increment_safe`
if you like, and update `racer()` to call it), then rebuild and run:

```
*** Booting Zephyr OS build v4.4.2 ***
shared_counter = 10 (expected 10)
```

Notice the `k_yield()` is still right there in the middle of the critical
section, doing exactly what it did before — handing control to the other
thread. The difference is what happens next: `racer2` immediately tries
to `k_mutex_lock()` too, but `racer1` still holds it, so `racer2` blocks
instead of reading `shared_counter` mid-update. `racer1` gets rescheduled,
finishes its write, unlocks — only then does `racer2` get to proceed. The
lock doesn't stop the scheduler from switching threads; it stops the
*data* from being touched by two threads at once.

## Try it yourself

- Increase `N_INCREMENTS` to something larger and confirm the locked
  version still always lands exactly on `2 * N_INCREMENTS`.
- Comment out just the `k_mutex_unlock()` call (keep the lock) and predict
  what happens before you rebuild and run it. (Hint: think about what a
  second thread calling `k_mutex_lock()` on an already-held, never-released
  mutex actually does.)
- Move the `k_yield()` so it happens *before* `k_mutex_lock()` instead of
  inside the critical section — does the bug come back? Why or why not?

Zephyr has other synchronization primitives for other jobs — semaphores
for signaling between threads (not just protecting data), condition
variables, spinlocks for very short critical sections. The
[Kernel Services](https://docs.zephyrproject.org/latest/kernel/services/index.html)
index links to all of them, under "Scheduling, Interrupts, and
Synchronization"; the mutex here is the one you'll reach for most often.

---
Next → [9. zbus](09-zbus.md)
