#!/usr/bin/env bash
#
# Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>
#
# SPDX-License-Identifier: Apache-2.0
#
# run-qemu.sh — boot a built Zephyr/ESP32 image in Espressif's QEMU fork.
#
# Why this script exists: there's no `west build -t run` support for real
# ESP32 boards (only for the qemu_* boards built into Zephyr itself). To run
# one in QEMU we have to build a flash image by hand: a zero-filled file the
# size of the board's flash chip, with our app written in starting at
# offset 0x1000 (the app's start address on ESP32 — flash offset 0 through
# 0xFFF is reserved). That's what the two `dd` calls below do.
#
# Usage: scripts/run-qemu.sh [build-dir] [flash-size-MB]
#   build-dir       defaults to "build"
#   flash-size-MB   defaults to 16 (m5stack_fire's flash size)

set -euo pipefail

BUILD_DIR="${1:-build}"
FLASH_SIZE_MB="${2:-16}"

BIN="$BUILD_DIR/zephyr/zephyr.bin"
if [ ! -f "$BIN" ]; then
	echo "error: $BIN not found — did you run 'west build' first?" >&2
	exit 1
fi

APP_OFFSET=4096 # 0x1000
FLASH_BYTES=$((FLASH_SIZE_MB * 1024 * 1024))
FLASH_IMAGE="$BUILD_DIR/flash.bin"

dd if=/dev/zero bs=1M count="$FLASH_SIZE_MB" of="$FLASH_IMAGE" status=none
dd if="$BIN" of="$FLASH_IMAGE" bs=1 seek="$APP_OFFSET" conv=notrunc status=none

echo "Booting $BIN in QEMU (Ctrl+A, X to quit)..."
exec qemu-system-xtensa -nographic -machine esp32 \
	-drive file="$FLASH_IMAGE",if=mtd,format=raw
