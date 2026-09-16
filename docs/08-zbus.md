<!--
Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>

SPDX-License-Identifier: Apache-2.0
-->

← Previous: [7. Synchronization: locks](07-synchronization.md)

# 8. zbus

[Step 7](07-synchronization.md) protected one shared `int` with a mutex
you had to remember to lock and unlock, correctly, every single time you
touched it. That's fine for one variable. It gets tedious — and easy to
get wrong — the moment a real application has many threads sharing many
pieces of data: a sensor reading here, a connection-state flag there, a
configuration struct somewhere else, each needing its own lock, each
publisher needing to know exactly who else cares about its data.

**zbus** is Zephyr's built-in publish/subscribe message bus: a structured
way for threads to share data where the publisher doesn't know or care
who's listening, listeners don't know or care who published, and zbus
handles the synchronization internally — no manual `k_mutex_lock()` at
every call site. The official
[zbus](https://docs.zephyrproject.org/latest/services/zbus/index.html)
documentation is the full reference; this step covers the two simplest
ways to receive.

## The channel

Look at [apps/zbus/src/main.c](../apps/zbus/src/main.c). A **channel**
carries one specific message type between however many publishers and
observers care about it:

```c
struct sensor_msg {
	int value;
};

ZBUS_CHAN_DEFINE(sensor_chan,                     /* channel name */
		  struct sensor_msg,                /* message type */
		  NULL,                             /* validator */
		  NULL,                             /* user data */
		  ZBUS_OBSERVERS(sensor_listener),  /* observers */
		  ZBUS_MSG_INIT(.value = 0)         /* initial value */
);
```

`ZBUS_OBSERVERS(...)` is the channel's static wiring — a list of names of
things (defined below) that get notified whenever someone publishes here.

## The listener

A **listener** is a plain callback function, invoked automatically every
time something publishes to its channel:

```c
static void sensor_listener_cb(const struct zbus_channel *chan)
{
	const struct sensor_msg *msg = zbus_chan_const_msg(chan);

	printk("listener saw value = %d\n", msg->value);
}

ZBUS_LISTENER_DEFINE(sensor_listener, sensor_listener_cb);
```

Nothing here calls `sensor_listener_cb()` directly — it's only ever
reached through zbus, because its name (`sensor_listener`) appears in the
channel's `ZBUS_OBSERVERS(...)` list above. Important nuance: a listener
runs **synchronously**, on the *publisher's own thread*, during the
publish call itself — not in a thread of its own. Keep listener callbacks
quick, for the same reason you'd keep an interrupt handler quick.

The publisher, meanwhile, doesn't know `sensor_listener` exists:

```c
static void publisher_entry(void *a, void *b, void *c)
{
	struct sensor_msg msg = { .value = 0 };

	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);

	while (1) {
		msg.value++;
		printk("publishing value = %d\n", msg.value);
		zbus_chan_pub(&sensor_chan, &msg, K_NO_WAIT);
		k_msleep(1000);
	}
}

K_THREAD_DEFINE(publisher_id, 1024, publisher_entry, NULL, NULL, NULL, 5, 0, 0);
```

## Build and run

```sh
# repo root
west build -b m5stack_fire/esp32/procpu -d build apps/zbus
./scripts/run-qemu.sh build 16
```

```
*** Booting Zephyr OS build v4.4.2 ***
publishing value = 1
listener saw value = 1
publishing value = 2
listener saw value = 2
...
```

Every `publishing` line is immediately followed by a matching `listener
saw` line — because, as above, the listener runs as part of the publish
call, not on its own schedule.

## Listeners vs. subscribers

A listener is convenient but limited: because it runs on the publisher's
thread, it can't block, and it can't decide to wait for a message —
zbus decides when it runs. A **subscriber** is the alternative: its own
thread, which explicitly waits for the next publish whenever *it's*
ready.

Add this to `apps/zbus/src/main.c` yourself:

```c
ZBUS_SUBSCRIBER_DEFINE(sensor_sub, 4);
ZBUS_CHAN_ADD_OBS(sensor_chan, sensor_sub, 3);

static void subscriber_entry(void *a, void *b, void *c)
{
	const struct zbus_channel *chan;
	struct sensor_msg msg;

	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);

	while (!zbus_sub_wait(&sensor_sub, &chan, K_FOREVER)) {
		zbus_chan_read(chan, &msg, K_MSEC(250));
		printk("subscriber woke up, value = %d\n", msg.value);
	}
}

K_THREAD_DEFINE(subscriber_id, 1024, subscriber_entry, NULL, NULL, NULL, 5, 0, 0);
```

`ZBUS_SUBSCRIBER_DEFINE` (instead of wiring the observer statically in
`ZBUS_OBSERVERS(...)`) plus `ZBUS_CHAN_ADD_OBS` adds it to the channel
afterward. `zbus_sub_wait()` blocks the subscriber's own thread until the
next publish, then `zbus_chan_read()` actually fetches the message.
Rebuild and run — now you'll see the ordering difference directly:

```
publishing value = 3
listener saw value = 3
subscriber woke up, value = 3
publishing value = 4
listener saw value = 4
subscriber woke up, value = 4
```

The listener always reports *inside* the same line group as `publishing`
(same thread, same instant); the subscriber's own thread gets scheduled
in afterward, however briefly later that turns out to be. Same channel,
same message, two genuinely different delivery mechanisms — pick whichever
fits what the receiver needs to do with the data.

## Try it yourself

- Add a second listener to the same channel and confirm both fire on
  every publish.
- Change the subscriber's queue size in `ZBUS_SUBSCRIBER_DEFINE(sensor_sub, 4)`
  to `1`, then publish faster than the subscriber can keep up (shrink the
  publisher's `k_msleep`) — what happens to messages that arrive before
  the subscriber catches up?
- Register a second channel (a different message struct entirely) and a
  listener that observes both — `zbus_chan_const_msg(chan)`'s return type
  depends on which channel actually fired, so you'll need to compare
  `chan` against each channel's address to know which one you're looking
  at.

---

This closes out the introduction. From here: the official
[Zephyr documentation](https://docs.zephyrproject.org/latest/) and the
samples under the Zephyr workspace's `zephyr/samples/` directory (see
[Step 2](02-directories.md)) are the natural next places to explore —
`samples/subsys/zbus/` in particular picks up right where this step left
off.
