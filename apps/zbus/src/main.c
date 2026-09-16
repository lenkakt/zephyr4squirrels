/*
 * Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * zbus
 *
 * A publisher thread simulates a sensor reading and publishes it to a
 * zbus channel every second. A listener -- registered as an observer of
 * that channel, not called directly by the publisher -- reacts to every
 * publish. Neither side holds a reference to the other; see
 * docs/08-zbus.md.
 */

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

struct sensor_msg {
	int value;
};

static void sensor_listener_cb(const struct zbus_channel *chan);

ZBUS_CHAN_DEFINE(sensor_chan,                     /* channel name */
		  struct sensor_msg,                /* message type */
		  NULL,                             /* validator */
		  NULL,                             /* user data */
		  ZBUS_OBSERVERS(sensor_listener),  /* observers */
		  ZBUS_MSG_INIT(.value = 0)         /* initial value */
);

static void sensor_listener_cb(const struct zbus_channel *chan)
{
	const struct sensor_msg *msg = zbus_chan_const_msg(chan);

	printk("listener saw value = %d\n", msg->value);
}

ZBUS_LISTENER_DEFINE(sensor_listener, sensor_listener_cb);

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
