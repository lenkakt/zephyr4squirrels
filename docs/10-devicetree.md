<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [9. zbus](09-zbus.md)

# 10. Devicetree

[Step 2](02-directories.md) mentioned devicetree twice in passing: every
board under the Zephyr workspace's `boards/` directory has one, and every
build produces a fully resolved copy at `build/zephyr/zephyr.dts`. This
step is about what's actually in there, and how your C code gets at it.

**Devicetree** is a text format that describes hardware: what chips,
buses, pins, and peripherals exist on a board, and how they're wired to
each other. Zephyr reads it at *build time*, not runtime — every
devicetree lookup you'll see below is resolved by the C preprocessor
before your code ever runs, so there's no runtime cost to asking "which
pin is this button on?".

## What's actually on the board

Open
[`m5stack_fire_procpu.dts`](https://github.com/zephyrproject-rtos/zephyr/blob/main/boards/m5stack/m5stack_fire/m5stack_fire_procpu.dts)
in the Zephyr workspace (`$ZEPHYR_BASE/boards/m5stack/m5stack_fire/`) — this
is the file that gets resolved into `build/zephyr/zephyr.dts` every time
you build for this board. Two parts of it matter here:

```dts
aliases {
	sw0 = &button0;
	...
};

buttons {
	compatible = "gpio-keys";

	button0: button_0 {
		gpios = <&gpio1 7 GPIO_ACTIVE_LOW>;
		label = "button left";
		zephyr,code = <INPUT_KEY_0>;
	};

	button1: button_1 { gpios = <&gpio1 6 GPIO_ACTIVE_LOW>; label = "button middle"; ... };
	...
};
```

The M5Stack Fire has four buttons in its devicetree (`button0`..`button3`),
but only the first one has a convenient short name: the `sw0` alias. The
other three exist and work exactly the same way — they just don't have an
alias yet. You'll fix that yourself further down.

## Reading a node, at compile time

Look at [apps/devicetree/src/main.c](../apps/devicetree/src/main.c).
`DT_ALIAS(sw0)` resolves the alias above to the `button0` node, entirely
in the preprocessor:

```c
#define SW0_NODE DT_ALIAS(sw0)

printk("sw0 label: %s\n", DT_PROP(SW0_NODE, label));
printk("sw0 pin:   %d\n", DT_GPIO_PIN(SW0_NODE, gpios));
```

`DT_PROP(node, label)` pulls the `label` string straight out of the
devicetree node. `DT_GPIO_PIN(node, gpios)` parses the `gpios =
<&gpio1 7 GPIO_ACTIVE_LOW>` phandle-array and extracts just the pin number
(`7`) — no driver, no device object, nothing happens at runtime. This is
just text substitution that happened before your code was even compiled.

## Binding a driver to the node

Macros alone don't let you *do* anything with the pin — for that you need
a driver bound to it:

```c
static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

if (!gpio_is_ready_dt(&sw0)) {
	printk("sw0 device not ready\n");
	return 0;
}

gpio_pin_configure_dt(&sw0, GPIO_INPUT);
printk("sw0 level: %d\n", gpio_pin_get_dt(&sw0));
```

`GPIO_DT_SPEC_GET` reads the same `gpios` property, but this time builds a
`struct gpio_dt_spec` — a real handle to the GPIO controller device plus
the pin and flags — that the GPIO driver can act on at runtime.
`gpio_pin_configure_dt()` and `gpio_pin_get_dt()` use it exactly the way
they'd use a spec you built by hand, except every number came straight
from devicetree instead of being hardcoded.

**A QEMU caveat:** `scripts/run-qemu.sh` boots a plain emulated ESP32 with
no button wired up to anything, so `sw0 level` will just print whatever
fixed value the emulated pin floats to — not a real press. The code above
is exactly what you'd run on the real M5Stack Fire, though, where pressing
the button changes that value live.

## Build and run

```sh
# repo root
west build -b m5stack_fire/esp32/procpu -d build apps/devicetree
./scripts/run-qemu.sh build 16
```

```
*** Booting Zephyr OS build v4.4.2 ***
sw0 label: button left
sw0 pin:   7
sw0 level: 1
```

## Writing your own overlay

An **overlay** is a small devicetree file that extends or overrides a
board's own `.dts` — without you ever touching the board file itself.
That matters because the board file lives in the Zephyr workspace, shared
by every app; an overlay lives *in your app*, next to `prj.conf`.

`button1`, `button2`, and `button3` exist on the board already, but have
no alias — so `DT_ALIAS(sw1)` won't compile yet. Fix that yourself:

Create `apps/devicetree/boards/m5stack_fire_esp32_procpu.overlay`:

```dts
/ {
	aliases {
		sw1 = &button1;
	};
};
```

The filename matters: Zephyr looks for `boards/<board>_<soc>_<core>.overlay`
matching the board target you built for (`m5stack_fire/esp32/procpu` →
`m5stack_fire_esp32_procpu`), and merges it into the board's devicetree
automatically — no CMake changes needed.

Now add the same read-and-bind pattern to
`apps/devicetree/src/main.c`, for `sw1` instead of `sw0`:

```c
#define SW1_NODE DT_ALIAS(sw1)

static const struct gpio_dt_spec sw1 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);

/* ... inside main(), after the sw0 block: */
printk("sw1 label: %s\n", DT_PROP(SW1_NODE, label));
if (gpio_is_ready_dt(&sw1)) {
	gpio_pin_configure_dt(&sw1, GPIO_INPUT);
	printk("sw1 level: %d\n", gpio_pin_get_dt(&sw1));
}
```

Rebuild — `sw1 label` should now print `button middle`, read straight out
of the node your overlay just aliased.

## Try it yourself

- Add `sw2` and `sw3` aliases the same way, for the two remaining buttons,
  and print all four labels and levels.
- Use your overlay to *override* a property instead of just adding an
  alias — for example, change `button0`'s `label` string and confirm your
  new text shows up instead of "button left".
- Look at `speaker` in the same `.dts` file (a `pwm-leds`-compatible node,
  not `gpio-keys`) — what would `DT_PROP(DT_NODELABEL(speaker), pwms)`
  even mean here? (Hint: it's not a single number — check what type the
  `pwms` property actually is in the devicetree bindings before guessing.)

---

That's the last step in this introduction. From here: the official
[Devicetree guide](https://docs.zephyrproject.org/latest/build/dts/index.html)
covers everything not touched on above (bindings, `#include`d `.dtsi`
files, the `&label { ... }` override syntax used all over the board file
you read from), and
[Zephyr documentation](https://docs.zephyrproject.org/latest/) plus the
samples under the Zephyr workspace's `zephyr/samples/` directory (see
[Step 2](02-directories.md)) are the natural next places to explore.
