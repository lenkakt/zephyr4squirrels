<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [3. Hello World in QEMU](03-hello-world.md)

# 4. West, the Zephyr meta-tool

You've already typed `west build ...` twice, in [Step 1](01-install.md) and
[Step 3](03-hello-world.md), without a real explanation of what it is. Let's fix that.

**West** is Zephyr's own meta-tool — its "Swiss Army knife," inspired by
tools like `git` and Google's `repo`. It manages the multi-repository
workspace (see [Step 2](02-directories.md)'s "topdir"), drives
configuration, builds, and (on real hardware) flashing the output binaries to the target, all through one
consistent command shape:

```
west [common-opts] <command> [opts] <args>
```

`west --help` lists every command; `west <command> --help` gives that
command's own options — e.g. `west build --help`.

## Where you already are

`west` always knows where your workspace's topdir is, no matter which
directory you run it from inside the workspace:

```sh
west topdir
```

That printed `/home/.../zephyrproject` when tested here — the same
directory [Step 2](02-directories.md) called the Zephyr workspace. This
repo (`zephyr4squirrels`) isn't itself part of that workspace; it's just
where your apps live, pointed at the workspace via the `ZEPHYR_BASE`
environment variable that [Step 1](01-install.md) had you set.

## Finding a board target

`west boards` lists every board Zephyr knows how to build for — all
**1,014 of them** (checked in september 2026), unsorted.
Searching beats scrolling:

```sh
west boards -n "m5stack_fire"
```

returns exactly one line, `m5stack_fire` — which means theat M5 Stack Fire board is supported.

## Pristine builds, and a real gotcha with them

[Step 3](03-hello-world.md) mentioned that repeating the same `west build`
command is near-instant (Ninja only rebuilds what changed), and that
`-p always` (or `--pristine always`) forces a clean rebuild by emptying the
build directory first. Verified here: a no-op rebuild took **0.3 seconds**
("`ninja: no work to do`"); the first build of the same app took **16
seconds**.

Here's the part that isn't obvious: a pristine build wipes out
*everything* CMake had cached about that build directory — including which
board you were building for. For example: if building `apps/hello_world`first for seeeduino board and than
for `m5stack_fire/esp32/procpu`, and then running

```sh
west build -d build -p always apps/hello_world   # -b left out on purpose
```

did **not** rebuild for `m5stack_fire` again. It silently built for
previously selected board  instead — whatever board happened to be configured as
this workspace's default (`west config -l` shows it as `build.board`).
No error, no warning, just a different board's image coming out the other
end.

**The fix is simple: always repeat `-b` alongside `-p`.**

```sh
west build -b m5stack_fire/esp32/procpu -d build -p always apps/hello_world
```

`-p auto` (heuristics decide if a pristine build looks necessary) and
`-p never` (fail instead of silently reusing an incompatible build
directory) are the other two settings — `auto` is a reasonable default,
but now you know why relying on it isn't quite the same as explicitly
saying which board you mean.

## Flashing — a note for later

`west flash` writes a built image to a *real, physically connected* board.
It has nothing to do with the QEMU workflow you've been using —
`scripts/run-qemu.sh` exists precisely because there's no flash step (and
no `west build -t run` support) for emulating a real hardware board like
`m5stack_fire`, as [Step 3](03-hello-world.md) explained. When this course
moves to the physical M5Stack Fire (or any other real physical device), `west flash` is what replaces
`run-qemu.sh` — same `build/` directory either way, just a different last
step.

## Try it yourself

- Run `west boards -n "esp32"` — how many boards come back? Skim a few
  names to get a feel for how many ESP32 variants Zephyr supports beyond
  just ours.
- Reproduce the pristine gotcha above yourself: build for
  `m5stack_fire/esp32/procpu`, then run a pristine rebuild *without* `-b`,
  and check the "Board:" line CMake prints near the start of the output.
- Run `west build --help` and find the `-p`/`--pristine` section — compare
  its wording to the explanation above.

---
Next → [5. The Zephyr shell](05-shell.md)
