<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [2. Important directories](02-directories.md)

# 3. Hello World in QEMU

If you followed [Step 1](01-install.md)'s "Verify it works" section, you've
already done this once. Let's slow down and actually understand what
happened.

## Build it

```sh
# repo root
west build -b m5stack_fire/esp32/procpu -d build apps/hello_world
```

- `-b m5stack_fire/esp32/procpu` — the board target. `m5stack_fire` is the
  board; `esp32/procpu` picks the ESP32's main core (it has two — `procpu`
  and `appcpu` — we only use one).
- `-d build` — where to put the build output (see [Step 2](02-directories.md)).
- `apps/hello_world` — the app to build; look at its
  [source](../apps/hello_world/src/main.c) if you haven't already, it's five
  lines.

The first build takes a minute or two (it compiles the kernel too, not just
your app). Run the same command again with nothing changed and it finishes
almost instantly — Zephyr's build system only rebuilds what changed. If you
switch board targets, add `-p always` (pristine) to force a clean rebuild.

## What is QEMU, exactly?

QEMU is a general-purpose machine emulator: it doesn't just run code, it
simulates an entire piece of hardware — the CPU, memory, flash, and
peripherals — closely enough that real firmware, built exactly the same
way as for a physical board, runs on it unmodified. The ESP32 uses a
Xtensa CPU, completely different from the x86_64 or ARM CPU in your own
laptop, so QEMU is doing genuine instruction-set translation under the
hood, not just running your CPU's instructions directly.

We use Espressif's own fork of QEMU (not the one bundled with the Zephyr
SDK) specifically because it models the ESP32's real peripherals — its
flash layout, boot ROM, and UART — accurately enough to boot the exact
same image that would run on the physical M5Stack Fire, completely
unmodified.

## Run it

```sh
# repo root
./scripts/run-qemu.sh build 16
```

Take a look at [scripts/run-qemu.sh](../scripts/run-qemu.sh) — it's short.
There's no `west build -t run` for real hardware boards like this one (that
shortcut only exists for Zephyr's own built-in `qemu_*` board targets), so
the script does by hand what's needed: pad a blank 16MB file to the size of
the board's flash chip, write your built image in starting at flash offset
`0x1000` (address `0x0`–`0xFFF` is reserved), then hand that file to
`qemu-system-xtensa` as an emulated flash chip.

You'll see something like:

```
Adding SPI flash device
ets Jul 29 2019 12:21:46
rst:0x1 (POWERON_RESET),boot:0x12 (SPI_FAST_FLASH_BOOT)
...
I (soc_init): ESP Simple boot
I (soc_init): chip revision: v0.0
E (soc_init): You are using ESP32 chip revision (0) that is unsupported. ...
I (flash_init): SPI Flash Size : 16MB
...
*** Booting Zephyr OS build v4.4.2 ***
Hello, squirrels! Zephyr is alive.
```

That's three separate things booting, in order:
1. **The ESP32's own ROM bootloader** (`ets Jul 29 2019...`) — real
   silicon-level boot code, faithfully emulated. The chip-revision warning
   is expected: QEMU emulates chip revision 0, an early/basic revision;
   `apps/hello_world/prj.conf` already has
   `CONFIG_ESP32_USE_UNSUPPORTED_REVISION=y` set so Zephyr proceeds anyway.
2. **Espressif's second-stage flash loader** (`soc_init`, `flash_init`) —
   reads the flash layout and jumps to your application.
3. **Zephyr itself** (`*** Booting Zephyr OS ***`) — finally, your `main()`
   runs.

## Why is this all just text?

Notice there's no window, no graphics — just a stream of text in your
terminal. That's not QEMU being minimal; that's genuinely what a
microcontroller looks like. There's no display, no desktop, no windowing
system anywhere in this picture. What almost every microcontroller does
have is a **UART**: a simple serial data line, meant for exactly this — a
basic, byte-at-a-time text link to another device.

Zephyr's console and `printk()` both default to writing to this UART. On
real hardware you'd connect a USB-to-serial adapter (or the board's
built-in one) and open a terminal program to watch it. In QEMU, the
`-nographic` flag in `run-qemu.sh` does the equivalent: instead of opening
a separate emulated "serial port window," it routes the emulated UART's
bytes straight into the terminal you're already sitting in. What you're
looking at when you see `Hello, squirrels!` *is* serial port traffic — the
same mechanism a real board uses — just piped to your own terminal instead
of a physical cable.

**A related note for later:** this isn't only about missing a GUI. Zephyr's
C library itself is a small, embedded-focused one (picolibc by default) —
not the full glibc/MSVCRT you're used to on Linux/Windows. Much of it works
exactly as you'd expect — `<math.h>` functions like `sqrt()` or `sin()`
compute correctly with no extra configuration — but things that assume a
full operating system underneath don't: file I/O beyond what a device
driver provides, `system()`, POSIX threads (Zephyr has its own thread API
instead), and GUI toolkits are either missing entirely or need specific
Kconfig options turned on.

C++ is similar, one level further: `CONFIG_CPP=y` alone gives you a bare
C++ language subset — no exceptions, no RTTI, and no standard library
headers at all (`<vector>`, `<string>`, `<iostream>` simply don't exist).
Turning on `CONFIG_GLIBCXX_LIBCPP=y` pulls in a real libstdc++, and
containers like `std::vector` and `std::string` then work normally — but
`<iostream>` (`std::cout`) still won't *link*, because it's built on POSIX
file-descriptor functions (`open`, `read`, `write`...) that Zephyr doesn't
provide by default. `printk()` has its own small version of this:
floating-point values print as the literal text `*float*` unless you
enable `CONFIG_CBPRINTF_FP_SUPPORT=y` — the math itself is always correct,
only the formatting code is opt-in, to save flash space on boards where
every kilobyte counts.

Press `Ctrl+A` then `X` to quit QEMU.

## Try it: change the message

Open [apps/hello_world/src/main.c](../apps/hello_world/src/main.c), change
the string in `printk(...)`, save, then rebuild and rerun:

```sh
# repo root
west build -b m5stack_fire/esp32/procpu -d build apps/hello_world
./scripts/run-qemu.sh build 16
```

You should see your new message instead. This build → run loop is what
you'll repeat constantly from here on — everything else in this course is
built on top of it.

---
Next → [4. West, the Zephyr meta-tool](04-west.md)
