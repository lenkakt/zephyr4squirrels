<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

# zephyr4squirrels

A short, hands-on introduction to [Zephyr OS](https://zephyrproject.org/),
for bachelor-level informatics students. No prior RTOS or embedded
experience assumed — just some C.

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/lenkakt/zephyr4squirrels)

Click the button above (or clone the repo and read locally — everything
here is plain Markdown) and work through these in order:

1. [What is Zephyr?](docs/00-what-is-zephyr.md)
2. [Install and set up Zephyr](docs/01-install.md)
3. [Important directories](docs/02-directories.md)
4. [Hello World in QEMU](docs/03-hello-world.md)
5. [The Zephyr shell](docs/04-shell.md)

No physical hardware is required — everything runs in an emulator (QEMU),
targeting the same chip (ESP32) as the course's real board, an
[M5Stack Fire](https://docs.zephyrproject.org/latest/boards/m5stack/m5stack_fire/doc/index.html).

## Repo layout

```
docs/           the five steps above
apps/           the Zephyr application(s) you'll build and edit
scripts/        run-qemu.sh — boots a built image in QEMU
.devcontainer/  the Codespaces environment definition
```
