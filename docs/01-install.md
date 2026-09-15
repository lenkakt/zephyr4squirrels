<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [0. What is Zephyr?](00-what-is-zephyr.md)

# 1. Install and set up Zephyr

You have two options. **Use GitHub Codespaces unless you have a specific
reason not to** — it gets you a working environment in a couple of clicks,
with nothing to install on your own machine.

## Option A: GitHub Codespaces (recommended)

**Already in a Codespace?** (e.g. you got here via the button on the
README) — you're done with this option already. Skip ahead to
[Verify it works](#verify-it-works) below.

Otherwise, click this button on the repo's GitHub page:

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/lenkakt/zephyr4squirrels)

That opens a cloud-hosted VS Code, in your browser, with a terminal already
inside this repository. The first launch takes a few minutes while the
container image is fetched — it comes with Zephyr, the ESP32 toolchain, and
the QEMU emulator already installed. You don't need to run `west update` or
install anything yourself; that's all baked into the image already.

You need a (free) GitHub account. Each Codespace draws on your own personal
monthly free quota, separate from everyone else's — see the note at the
bottom of this page if you're curious about the details.

Once your Codespace's terminal is ready, skip to [Verify it works](#verify-it-works)
below.

## Option B: Install locally

If you'd rather work on your own machine (Linux, macOS, or Windows via
WSL2), follow these steps. This mirrors exactly what the Codespaces image
does under the hood — see [.devcontainer/Dockerfile](../.devcontainer/Dockerfile)
if you want the literal source of truth.

### 1. Install prerequisites

On Ubuntu/Debian:

```sh
sudo apt update
sudo apt install --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget python3-dev python3-pip \
  python3-venv python3-setuptools python3-tk python3-wheel xz-utils file \
  make gcc g++ gcc-multilib g++-multilib libsdl2-dev libmagic1 \
  libpixman-1-0 libslirp0
```

(macOS/Windows: follow the package list in the official
[Install dependencies](https://docs.zephyrproject.org/latest/develop/getting_started/index.html#install-dependencies)
guide — the rest of these steps are the same everywhere.)

### 2. Clone this repo

```sh
# ~
git clone https://github.com/lenkakt/zephyr4squirrels ~/zephyr4squirrels
cd ~/zephyr4squirrels
```

The rest of these steps assume you're inside this directory.

### 3. Set up a Python environment and install west

Kept inside the repo (`.venv`, already in `.gitignore`) rather than
somewhere in your home directory, so it can't collide with a virtualenv
from some other project:

```sh
# ~/zephyr4squirrels
python3 -m venv .venv
source .venv/bin/activate
pip install west
```

Remember to `source .venv/bin/activate` again (from this directory) in
every new terminal.

### 4. Fetch Zephyr

```sh
# ~/zephyr4squirrels
git clone --depth 1 --branch v4.4.2 https://github.com/zephyrproject-rtos/zephyr ~/zephyrproject/zephyr
cd ~/zephyrproject/zephyr
# ~/zephyrproject/zephyr
west init -l --mf west.yml .
cd ~/zephyrproject
# ~/zephyrproject
west update -o=--depth=1 -n
west zephyr-export
pip install -r zephyr/scripts/requirements.txt
west packages pip --install
```

We pin to `v4.4.2` (not `main`) so the course doesn't shift under you
mid-semester.

### 5. Install the ESP32 toolchain

```sh
# ~/zephyrproject
cd ~/zephyrproject/zephyr
# ~/zephyrproject/zephyr
west sdk install --gnu-toolchains xtensa-espressif_esp32_zephyr-elf \
  --install-dir ~/zephyr-sdk
```

Set these environment variables in every new terminal (or add them to your
shell's startup file):

```sh
export ZEPHYR_BASE=~/zephyrproject/zephyr
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk
```

### 6. Install Espressif's ESP32 QEMU fork

We use Espressif's own QEMU fork, not the one bundled with the Zephyr SDK —
it has much better ESP32 hardware fidelity.

```sh
mkdir -p ~/espressif-qemu
wget -O /tmp/esp-qemu.tar.xz \
  "https://github.com/espressif/qemu/releases/download/esp-develop-9.2.2-20260417/qemu-xtensa-softmmu-esp_develop_9.2.2_20260417-x86_64-linux-gnu.tar.xz"
tar -xJf /tmp/esp-qemu.tar.xz -C ~/espressif-qemu --strip-components=1
export PATH="$HOME/espressif-qemu/bin:$PATH"
```

(On macOS, swap the asset filename for the `x86_64-apple-darwin` or
`aarch64-apple-darwin` one from the same
[release page](https://github.com/espressif/qemu/releases).)

## Verify it works

From this repo's root (in your Codespace terminal, or locally with the
environment variables from step 5 above still set):

```sh
# repo root — if you followed Option B, cd ~/zephyr4squirrels first
west build -b m5stack_fire/esp32/procpu -d build apps/hello_world
./scripts/run-qemu.sh build 16
```

That second command boots your build in **QEMU** — an emulator that
simulates the ESP32 chip closely enough to run real firmware unmodified.
More on what QEMU actually is in [Step 3](03-hello-world.md#what-is-qemu-exactly).

You should see the ESP32 boot log, then:

```
*** Booting Zephyr OS build v4.4.2 ***
Hello, squirrels! Zephyr is alive.
```

Press `Ctrl+A` then `X` to quit QEMU. If you see that line, you're set up
correctly — move on to the next step.

<details>
<summary>Curious about Codespaces cost/limits?</summary>

GitHub Codespaces usage is billed per individual GitHub account, not
shared — your classmates opening their own Codespaces has no effect on
yours. Personal (free) GitHub accounts get a monthly allowance of compute
hours and storage; on the small machine this repo requests, that's plenty
for course use. No credit card is required unless you exceed your own free
allowance.
</details>

---
Next → [2. Important directories](02-directories.md)
