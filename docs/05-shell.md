<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [4. West, the Zephyr meta-tool](04-west.md)

# 5. The Zephyr shell

So far `apps/hello_world` prints one line and stops. Real firmware often
needs a way to poke at a running system interactively — inspect memory,
check what threads are doing, call into your own code — without attaching
a debugger. That's what Zephyr's **shell** subsystem is for.

It's not a Unix shell: there's no filesystem, no external programs. It's a
fixed set of commands, each one a C function, registered into the firmware
image at build time. What you get is exactly what you (and Zephyr) chose
to compile in — nothing more.

## Turn it on

Add one line to [apps/hello_world/prj.conf](../apps/hello_world/prj.conf):

```
CONFIG_SHELL=y
```

Rebuild and run as before:

```sh
# repo root
west build -b m5stack_fire/esp32/procpu -d build apps/hello_world
./scripts/run-qemu.sh build 16
```

After the usual boot log and your hello-world message, you'll now land on
a prompt instead of QEMU just sitting there:

```
uart:~$
```

Press `Tab` on an empty line to list every available command:

```
  clear     : Clear screen.
  date      : Date commands
  device    : Device commands
  devmem    : Read/write physical memory
  help      : Prints the help message.
  history   : Command history.
  kernel    : Kernel commands
  rem       : Ignore lines beginning with 'rem '
  resize    : Console...
  retval    : Print return value of most recent command
  shell     : Useful, not Unix-like shell commands.
```

Try a couple:

```
uart:~$ device list
devices:
- clock (READY)
  DT node labels: clock
- gpio@3ff44800 (READY)
  DT node labels: gpio1
- gpio@3ff44000 (READY)
  DT node labels: gpio0
- uart@3ff40000 (READY)
  DT node labels: uart0
- buttons (READY)

uart:~$ kernel thread list
Scheduler: 32373 since last call
Threads:
 0x3ffb2cb8 input
	options: 0x0, priority: 14 timeout: 0
	state: pending, ...
*0x3ffb2b58 shell_uart
	options: 0x0, priority: 14 timeout: 0
	state: queued, ...
 0x3ffb2fb8 sysworkq
	...
 0x3ffb2e78 idle
	...
```

`kernel thread list` is worth sitting with for a second: you're looking at
every thread currently running in *your* firmware, including the shell
itself (`shell_uart`) and the idle thread — real insight into a live
system, from one command.

## Add your own command

Registering a shell command is one macro call. Add this to
[apps/hello_world/src/main.c](../apps/hello_world/src/main.c), above `main()`:

```c
#include <zephyr/shell/shell.h>

static int cmd_squirrel(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, "A squirrel appears and steals your bandwidth.");
	return 0;
}

SHELL_CMD_REGISTER(squirrel, NULL, "Summon a squirrel.", cmd_squirrel);
```

Rebuild, run, and try it:

```
uart:~$ squirrel
A squirrel appears and steals your bandwidth.
```

`SHELL_CMD_REGISTER` takes: the command's name, a subcommand table (`NULL`
— we have none), a one-line help string (shown by `help` and tab-complete),
and the function to call. The function receives `argc`/`argv` like a
regular `main()`, if your command needs arguments.

## `SHELL_CMD_REGISTER` is a macro, not a function

Notice `SHELL_CMD_REGISTER(...)` isn't called from inside `main()` or any
function — it sits at file scope, on its own. That's because it's a
**macro**, not a function call: it expands, at compile time, into a small
static data structure (holding your command's name, help text, and a
pointer to `cmd_squirrel`) placed somewhere the shell subsystem knows to
look for it at boot. There's no central "list of commands" you have to
maintain by hand anywhere — every `SHELL_CMD_REGISTER` anywhere in the
build contributes one entry, automatically.

This is a pattern worth recognizing, because Zephyr uses it constantly, far
beyond the shell: you give it a name, a small piece of your own code (a
callback function), and a few parameters — and a macro does the wiring
that would otherwise be repetitive boilerplate. `K_THREAD_DEFINE(...)`
declares and starts a thread the same way; `SYS_INIT(...)` registers a
function to run automatically during boot, at a priority you choose;
device drivers register themselves with a similar macro so the kernel
finds them without any of your own code calling into a central registry.
Once you recognize "name + callback + a few parameters, wrapped in a
macro" as a shape, a lot of unfamiliar corners of the Zephyr API stop
looking unfamiliar.

## Try it yourself

- Make `squirrel` take an argument (check `argv[1]` when `argc > 1`) and
  print something different depending on it.
- Register a second command that calls `k_uptime_get()` and prints how long
  the board's been running.
- Look at `kernel thread list`'s stack usage numbers, then compare them
  after adding a few more commands — does registering more shell commands
  change any thread's own stack usage? (It shouldn't — think about why.)

---
Next → [6. Threads, basics](06-threads-basics.md)
