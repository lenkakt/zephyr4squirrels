# Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>
#
# SPDX-License-Identifier: Apache-2.0

# A lean, from-scratch image: only the pieces this course actually needs
# (one board, one architecture), instead of building on the Zephyr
# Project's own all-purpose CI image, which bundles toolchains for every
# architecture Zephyr supports plus tools (Renode, ARM FVPs, LLVM, ...) we
# never touch here. That image is ~23GB on its own; this one is a fraction
# of that. See plan.md for the size investigation that led here.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
	git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget \
	python3-dev python3-pip python3-venv python3-setuptools python3-tk python3-wheel \
	xz-utils file make gcc g++ gcc-multilib g++-multilib libsdl2-dev libmagic1 \
	curl ca-certificates sudo \
	libpixman-1-0 libslirp0 \
	&& rm -rf /var/lib/apt/lists/*

# ubuntu:24.04 already ships a "ubuntu" user at uid 1000 (matches the uid
# GitHub Codespaces' own default user uses) — use it directly rather than
# creating a second one and hitting a "UID not unique" conflict.
RUN echo "ubuntu ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/ubuntu
USER ubuntu
WORKDIR /home/ubuntu

# An isolated Python environment for west and Zephyr's own Python deps.
RUN python3 -m venv /home/ubuntu/.venv
ENV PATH="/home/ubuntu/.venv/bin:${PATH}"
RUN pip install --no-cache-dir west

# --- A ready-to-build Zephyr workspace, baked into the image ---
# Kept outside the repo's own mount point (the repo becomes the Codespace's
# workspaceFolder separately, at container start) so students never wait on
# `west update` themselves. Pinned to a specific release, not `main`, so
# the course doesn't shift under students mid-semester.
ARG ZEPHYR_VERSION=v4.4.2
ENV ZEPHYR_BASE=/home/ubuntu/zephyrproject/zephyr

RUN git clone --depth 1 --branch ${ZEPHYR_VERSION} \
	https://github.com/zephyrproject-rtos/zephyr /home/ubuntu/zephyrproject/zephyr \
	&& cd /home/ubuntu/zephyrproject/zephyr && west init -l --mf west.yml . \
	&& cd /home/ubuntu/zephyrproject && west update -o=--depth=1 -n \
	&& west zephyr-export \
	&& pip install --no-cache-dir -r zephyr/scripts/requirements.txt \
	&& west packages pip --install

# --- Only the one toolchain this course targets ---
# (Not `west sdk install` with no --gnu-toolchains, which pulls every
# architecture's compiler — dozens of GB we'd never use.)
ENV ZEPHYR_SDK_INSTALL_DIR=/home/ubuntu/zephyr-sdk
RUN cd /home/ubuntu/zephyrproject/zephyr \
	&& west sdk install --gnu-toolchains xtensa-espressif_esp32_zephyr-elf \
	--install-dir ${ZEPHYR_SDK_INSTALL_DIR}

# --- Espressif's ESP32 QEMU fork ---
# Not the QEMU bundled with the Zephyr SDK: Espressif maintains its own
# fork with better ESP32 hardware fidelity, which is what actually lets us
# boot Zephyr on an emulated ESP32. See https://github.com/espressif/qemu
ARG ESP_QEMU_RELEASE=esp-develop-9.2.2-20260417
ARG ESP_QEMU_ASSET=qemu-xtensa-softmmu-esp_develop_9.2.2_20260417-x86_64-linux-gnu.tar.xz
RUN mkdir -p /home/ubuntu/espressif-qemu \
	&& wget -q -O /tmp/esp-qemu.tar.xz \
	"https://github.com/espressif/qemu/releases/download/${ESP_QEMU_RELEASE}/${ESP_QEMU_ASSET}" \
	&& tar -xJf /tmp/esp-qemu.tar.xz -C /home/ubuntu/espressif-qemu --strip-components=1 \
	&& rm /tmp/esp-qemu.tar.xz
ENV PATH="/home/ubuntu/espressif-qemu/bin:${PATH}"
