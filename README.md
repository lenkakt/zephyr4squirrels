<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

# zephyr4squirrels

A short, hands-on introduction to [Zephyr OS](https://zephyrproject.org/),
for bachelor-level informatics students. No prior RTOS or embedded
experience assumed — just some C.

The original student group this tutorial was designed for uses an M5Stack Fire in their labs — that's why the whole tutorial is built around emulating this exact device, to stay as close as possible to what they'll use later.

The tutorial was created and tested for Zephyr 4.2. If you run it in Codespaces,
it runs on an Ubuntu Linux base. The local-install instructions are written for
any current mainstream Linux distribution.

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/lenkakt/zephyr4squirrels)

**Not in a Codespace yet?** Click the button above — it opens one for you.
Building it for the first time may take several minutes.

**Already inside a Codespace, or reading this locally after cloning?**
You're set — ignore the button and jump straight to the first section
below.

Work through these in order:

1. [What is Zephyr?](docs/00-what-is-zephyr.md)
2. [Install and set up Zephyr](docs/01-install.md)
3. [Important directories](docs/02-directories.md)
4. [Hello World in QEMU](docs/03-hello-world.md)
5. [The Zephyr shell](docs/04-shell.md)
6. [Threads, basics](docs/05-threads-basics.md)
7. [Threads, extended](docs/06-threads-extended.md)
8. [Synchronization: locks](docs/07-synchronization.md)
9. [zbus](docs/08-zbus.md)

No physical hardware is required — everything runs in an emulator (QEMU),
targeting the same chip (ESP32) as the course's real board, an
[M5Stack Fire](https://docs.zephyrproject.org/latest/boards/m5stack/m5stack_fire/doc/index.html).

## Repo layout

```
docs/           the steps above
apps/           the Zephyr application(s) you'll build and edit
scripts/        run-qemu.sh — boots a built image in QEMU
.devcontainer/  the Codespaces environment definition
```
