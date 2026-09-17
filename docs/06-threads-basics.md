<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [5. The Zephyr shell](05-shell.md)

# 6. Threads, basics

Every application so far has really been one thread: the kernel runs your
`main()` in a thread of its own (the "main thread"), plus whatever a
subsystem adds behind the scenes — recall `kernel thread list` back in
[Step 5](05-shell.md): you were already looking at more than one thread,
you just weren't creating any yourself.

Real embedded programs are rarely single-threaded. A sensor might need
reading every 100ms while a network link is serviced once a second,
independently, without either blocking the other. That's what threads are
for: each one is an independent execution context, with its own stack, a
priority, and its own idea of what it's doing — scheduled by the kernel,
not by you. The official
[Kernel Services: Threads](https://docs.zephyrproject.org/latest/kernel/services/threads/index.html)
documentation is the complete reference for everything in this stage and
the next one — worth bookmarking.

## The code

Look at [apps/threads/src/main.c](../apps/threads/src/main.c):

```c
static void print_to_shell(const char *name)
{
	printk("%s\n", name);
}

static void thread_entry(void *name, void *period_ms, void *unused)
{
	ARG_UNUSED(unused);

	while (1) {
		print_to_shell((const char *)name);
		k_msleep((uint32_t)(uintptr_t)period_ms);
	}
}

K_THREAD_DEFINE(thread1_id, STACKSIZE, thread_entry,
		 "Thread 1", (void *)1000, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(thread2_id, STACKSIZE, thread_entry,
		 "Thread 2", (void *)700, NULL, PRIORITY, 0, 0);
```

Notice there's only **one** thread function, `thread_entry`, used by
**two** threads. This is the same registration-macro pattern from
[Step 5](05-shell.md#shell_cmd_register-is-a-macro-not-a-function):
`K_THREAD_DEFINE` doesn't call your function — it declares and starts a
thread that will call it, and hands it whatever parameters you gave the
macro. Here that's a name (`"Thread 1"` / `"Thread 2"`) and a period
(`1000` / `700`, in milliseconds), both passed through as plain `void *`
and cast back inside the function. Same code, two independent threads,
each with its own idea of who it is.

`K_THREAD_DEFINE`'s other two arguments matter too:
- **Stack size** (`STACKSIZE`, 1024 bytes here) — each thread gets its own
  private, fixed-size memory region for local variables and function
  calls, reserved up front. Get it wrong and things go badly — enough that
  [Step 7](07-threads-extended.md) is entirely about it.
- **Priority** (`PRIORITY`, 5 for both) — lower numbers run first when
  more than one thread is ready to run at the same time. Both threads
  here share the same priority, so neither can starve the other; the
  scheduler just runs whichever one's `k_msleep()` woke up first.

## Build and run

This is the first time in the tutorial you're building a *different* app
than the one before it — and Zephyr's build system remembers which app and
board a build directory belongs to. Since Steps 3–4 both used `-d build`
for `apps/hello_world`, pointing that same directory at `apps/threads`
without telling it to start over gets you:

```
ERROR: Build directory "/home/user/zephyr4squirrels/build" is for application
"/home/user/zephyr4squirrels/apps/hello_world", but source directory
"/home/user/zephyr4squirrels/apps/threads" was specified; please clean it,
use --pristine, or use --build-dir to set another build directory
FATAL ERROR: refusing to proceed without --force due to above error
```

Fix it with `-p always` (pristine), which forces a clean rebuild in that
directory — that's why the command below includes it:

```sh
# repo root
west build -b m5stack_fire/esp32/procpu -d build -p always apps/threads
./scripts/run-qemu.sh build 16
```

Only use `-p always` when you're actually switching apps or boards, though
— it rebuilds everything from scratch, kernel included, which is
noticeably slower than a normal incremental build. Building `apps/threads`
again with nothing changed doesn't need it.

After the usual boot log, both threads start printing, forever:

```
Thread 1
Thread 2
Thread 2
Thread 1
Thread 2
Thread 1
Thread 2
Thread 2
Thread 1
...
```

That's not a typo — look closely at the pattern. It's not a clean
one-for-one alternation, because the two threads genuinely don't know
about each other: Thread 1 wakes up every 1000ms, Thread 2 every 700ms,
and the scheduler just runs whichever one's timer fires first. Press
`Ctrl+A` then `X` to quit QEMU.

## Try it yourself

- Change one of the periods (or both) and watch the interleaving pattern
  change. Try making them equal — does the output become perfectly
  alternating, or does it stay unpredictable? Why?
- Add `CONFIG_SHELL=y` to [apps/threads/prj.conf](../apps/threads/prj.conf),
  like you did in Step 5. The default periods print too fast to leave you
  room to type, so bump both up to a few seconds first. Rebuild and run
  `kernel thread list` — your two threads now show up right alongside the
  kernel's own.
- Add a third `K_THREAD_DEFINE` reusing the same `thread_entry` function,
  with its own name and period.
- Give the two threads *different* priorities and see if you can tell any
  difference in the output. (You likely can't — think about why: both
  threads spend almost all their time asleep in `k_msleep()`, not
  competing for the CPU, so priority has nothing to arbitrate here. It
  would matter if both threads were doing real work back-to-back instead.)

---
Next → [7. Threads, extended](07-threads-extended.md)
