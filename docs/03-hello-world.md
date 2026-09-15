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
switch board targets, add `-p` (pristine) to force a clean rebuild.

## Run it

```sh
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

Press `Ctrl+A` then `X` to quit QEMU.

## Try it: change the message

Open [apps/hello_world/src/main.c](../apps/hello_world/src/main.c), change
the string in `printk(...)`, save, then rebuild and rerun:

```sh
west build -b m5stack_fire/esp32/procpu -d build apps/hello_world
./scripts/run-qemu.sh build 16
```

You should see your new message instead. This build → run loop is what
you'll repeat constantly from here on — everything else in this course is
built on top of it.

---
Next → [4. The Zephyr shell](04-shell.md)
