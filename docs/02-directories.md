<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [1. Install and set up Zephyr](01-install.md)

# 2. Important directories

Before writing any code, it helps to know where things actually live —
there are two separate locations, and it's easy to conflate them.

## Two separate places

1. **The Zephyr workspace** — the Zephyr OS source code itself, plus every
   hardware-vendor HAL it can target. This is big (a few GB), shared,
   and you'll rarely open a file in it directly. It's already set up for
   you:
   - In Codespaces: `/home/ubuntu/zephyrproject`
   - Local install: wherever you cloned it in [Step 1](01-install.md) (`~/zephyrproject` in the example there)

   `echo $ZEPHYR_BASE` always tells you where the `zephyr/` part of it is.

2. **This repo** (`zephyr4squirrels`) — your own code: the small apps you
   build and edit, and these docs. This is what you actually work in.
   - In Codespaces: `/workspaces/zephyr4squirrels` (opened for you automatically)
   - Local install: wherever you cloned it (`~/zephyr4squirrels` in the example)

Every `west build` command you run points at an app *in this repo*, but
uses the kernel and drivers *from the workspace*. Two places, one build.

## Inside the Zephyr workspace, briefly

```
zephyrproject/            # topdir — the workspace root
├── .west/config           # west's own settings for this workspace
└── zephyr/                # the actual Zephyr OS source
    ├── kernel/             # the RTOS kernel itself
    ├── drivers/            # driver implementations, by subsystem
    ├── boards/              # board definitions (devicetree + Kconfig per board)
    ├── samples/             # official example applications
    └── scripts/west_commands/  # what implements `west build`, `west flash`, etc.
```

You won't need to edit anything under `zephyr/` in this course — but it's
worth knowing `samples/` exists; it's full of working examples you can
learn from later, beyond what we cover here.

## Inside an application (this repo)

Every app in this repo, e.g. [apps/hello_world/](../apps/hello_world/),
has the same three-file skeleton:

```
apps/hello_world/
├── CMakeLists.txt   # boilerplate: locate Zephyr, declare your source files
├── prj.conf         # Kconfig options: which kernel features are enabled
└── src/main.c        # your code
```

- **`CMakeLists.txt`** always starts the same way —
  `find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})` — which is the one
  line that pulls in Zephyr's entire build system and connects this small
  app to the big workspace next to it.
- **`prj.conf`** is a flat list of `CONFIG_*=y` lines: how you turn kernel
  features on without writing any C. `hello_world`'s only sets
  `CONFIG_PRINTK=y` and the ESP32 QEMU compatibility option you saw in
  Step 1.
- **`src/main.c`** is plain C, with `#include <zephyr/kernel.h>` giving you
  the kernel API.

After you build (`west build -b ... -d build apps/hello_world`), a fourth
directory appears — `build/`, created fresh each time, holding everything
the build produced:

```
build/
├── .config              # the fully resolved Kconfig — your prj.conf plus every default
└── zephyr/
    ├── zephyr.elf         # the final linked image
    ├── zephyr.bin          # same, in raw binary form — what run-qemu.sh boots
    └── zephyr.dts          # the fully resolved devicetree for this board
```

`build/` is disposable — delete it any time and `west build` regenerates it.

---
Next → [3. Hello World in QEMU](03-hello-world.md)
