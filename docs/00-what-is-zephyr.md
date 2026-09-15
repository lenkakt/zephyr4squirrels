<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

# 0. What is Zephyr?

Zephyr is a small, scalable **real-time operating system (RTOS)** for microcontrollers and other resource-constrained connected devices. It's open source (Apache-2.0), hosted by the Linux Foundation, and backed by most major chip vendors (Nordic, NXP, Intel, ST, TI, and more).

**It is not "little Linux."** They share ideas — both use Kconfig and Devicetree for configuration — but a Zephyr application is compiled *into* the OS as one static binary, usually with no virtual memory or process isolation, running in kilobytes of RAM. Real-time behavior is built in, not bolted on.

**Crucial parts you'll meet in this course:**
- **West** — the meta-tool: fetches source, builds, flashes.
- **Kconfig** — turns features/drivers on or off at build time, via `prj.conf`.
- **Devicetree** — a text description of the hardware a driver runs against.
- **CMake** — glues it all together into one binary.

No hardware required to start — everything here runs first in **QEMU**, which is standard engineering practice, not just a fallback.

---
Next → [1. Install and set up Zephyr](01-install.md)
