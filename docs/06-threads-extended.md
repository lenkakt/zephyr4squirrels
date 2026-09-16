<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [5. Threads, basics](05-threads-basics.md)

# 6. Threads, extended

[Step 5](05-threads-basics.md) got two threads running. This step covers
two things you'll run into the moment you write a thread that does
anything more than print its own name: sizing its stack correctly, and
getting more than three pieces of data into it. See also the official
[Kernel Services: Threads](https://docs.zephyrproject.org/latest/kernel/services/threads/index.html)
documentation for the full reference.

## What's actually on the stack

A thread's stack isn't just "memory for local variables." Every function
call — including ones you didn't write, like `k_msleep()` or `printk()`
internally calling other functions — pushes a **stack frame**: a return
address, some saved registers, and that function's local variables. Call a
function from inside a function, and its frame stacks on top of the
caller's. A local array or struct declared inside any of those functions
lives on the stack too, for as long as that function is still running.

`STACKSIZE` reserves a fixed region for all of that, per thread, up front.
Get it wrong in either direction and it costs you:
- **Too small**, and the *next* push after the region fills up doesn't
  fail cleanly — it just writes into whatever memory happens to sit next
  to it. That might be another thread's stack, a global variable, or
  memory nothing's using yet. The actual crash, when it comes, is often
  somewhere else entirely, and confusing to trace back.
- **Too big**, and you're not "safer" — you've just reserved RAM, forever,
  whether the thread ever uses it or not. Recall from
  [Step 0](00-what-is-zephyr.md): this board has kilobytes, not gigabytes.
  Across several threads, over-provisioned stacks add up fast.

You don't have to guess: `kernel thread list` — the same shell command
from [Step 4](04-shell.md) — reports each thread's stack usage as a
percentage. Enable `CONFIG_SHELL=y` on `apps/threads`, and while you're at
it, bump both periods up to a few seconds (e.g. `5000`/`3000`) — at the
default 700ms/1000ms, the threads print too fast to leave you room to type
a command. Rebuild, and go check the numbers now; you'll find both threads
are using only a small fraction of their 1024 bytes for a job this simple.

### What it looks like when you get it wrong

Shrink `apps/threads`' `STACKSIZE` down to `64` and rebuild. Here's what
that actually produces:

```
*** Booting Zephyr OS build v4.4.2 ***
 ** FATAL EXCEPTION
 ** CPU 0 EXCCAUSE 15 (load/store PIF addr error)
 ** VADDR 0xfffffff0 Invalid SP 0x3ffe7fa4
```

Notice this is *not* a clean "stack overflow detected" message — it's a
raw CPU exception, because nothing here explicitly enabled stack-overflow
detection. Zephyr does offer mechanisms for this (`CONFIG_STACK_SENTINEL`,
or hardware-assisted protection on architectures that support it), but
they have to be turned on, and even then aren't a guarantee of a clean
diagnosis for every kind of overflow. That's exactly why the previous
section's advice is the real fix, not a fallback: check `kernel thread
list`'s usage numbers proactively, rather than waiting to hit a crash like
this one and having to work backward from a raw fault address.

## More than three parameters: pass a struct pointer

`K_THREAD_DEFINE` gives every thread exactly three parameter slots —
`p1`, `p2`, `p3` — because that's the shape baked into the macro. When you
need to hand a thread more than three pieces of information (or something
that isn't already pointer-sized), the standard trick is: bundle
everything into a `struct`, and pass a **pointer** to it as `p1`.

```c
#include <zephyr/kernel.h>

#define STACKSIZE 1024
#define PRIORITY  5

struct thread_config {
	const char *name;
	uint32_t period_ms;
	int repeat_count;
};

static struct thread_config configs[] = {
	{ "Thread 1", 1000, 3 },
	{ "Thread 2", 700, 3 },
};

static void thread_entry(void *config_ptr, void *unused2, void *unused3)
{
	struct thread_config *cfg = (struct thread_config *)config_ptr;

	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	for (int i = 0; i < cfg->repeat_count; i++) {
		printk("%s (%d/%d)\n", cfg->name, i + 1, cfg->repeat_count);
		k_msleep(cfg->period_ms);
	}
	printk("%s done\n", cfg->name);
}

K_THREAD_DEFINE(thread1_id, STACKSIZE, thread_entry, &configs[0], NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(thread2_id, STACKSIZE, thread_entry, &configs[1], NULL, NULL, PRIORITY, 0, 0);
```

One pointer, three fields — and nothing stops that struct from having ten
fields instead of three. Build and run this (try it as a modification to
`apps/threads`, or ask for it as a separate app) and you'll see:

```
*** Booting Zephyr OS build v4.4.2 ***
Thread 1 (1/3)
Thread 2 (1/3)
Thread 2 (2/3)
Thread 1 (2/3)
Thread 2 (3/3)
Thread 1 (3/3)
Thread 2 done
Thread 1 done
```

Each thread reads only its own config through its own pointer — `configs[0]`
and `configs[1]` never get mixed up, even though both threads are running
the exact same `thread_entry` code.

Unlike every thread you've seen so far, these two don't loop forever —
`thread_entry` returns once `repeat_count` is reached, and a thread that
returns is done, permanently. With `CONFIG_SHELL=y` on, run `kernel thread
list` *after* both threads have printed "done": they're not just idle,
they're gone from the list entirely, leaving only the kernel's own
threads (`shell_uart`, `sysworkq`, `idle`, ...) behind.

## Try it yourself

- Rebuild `apps/threads` with the struct-pointer version above. Change
  `repeat_count` per thread and confirm each one stops independently.
- Try `CONFIG_STACK_SENTINEL=y` on the `STACKSIZE=64` build from earlier —
  does the failure look any different? (Don't assume it'll be clean —
  that's the point.)
- Add a fourth field to `thread_config` (e.g. a starting delay before the
  loop begins) and use it.
- Go back to the original, infinite-loop version of `apps/threads` (the
  one from [Step 5](05-threads-basics.md), still printing forever) with
  `CONFIG_SHELL=y` on. Run `kernel thread list`, note the address shown
  next to `thread1_id`, then kill it yourself:
  `kernel thread kill <that address>`. The thread stops printing
  immediately, and a second `kernel thread list` shows it gone — you just
  did from the shell what `repeat_count` did automatically a moment ago.

---
Next → [7. Synchronization: locks](07-synchronization.md)
