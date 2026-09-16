/*
 * Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * sync
 *
 * Two threads increment the same shared counter, unprotected. The
 * increment is deliberately split into a read, a k_yield() (to force a
 * preemption right in the middle -- widening a race that's real either
 * way, just usually rarer and less convenient to reproduce on demand),
 * and a write. Expect this to print a wrong total; see docs/07-synchronization.md.
 */

#include <zephyr/kernel.h>

#define STACKSIZE     1024
#define PRIORITY      5
#define N_INCREMENTS  5

static int shared_counter;

static void increment_unsafe(void)
{
	int temp = shared_counter;

	k_yield();
	shared_counter = temp + 1;
}

static void racer(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	for (int i = 0; i < N_INCREMENTS; i++) {
		increment_unsafe();
	}
}

K_THREAD_DEFINE(racer1_id, STACKSIZE, racer, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(racer2_id, STACKSIZE, racer, NULL, NULL, NULL, PRIORITY, 0, 0);

int main(void)
{
	/* Give both racer threads time to finish before we read the result. */
	k_msleep(500);
	printk("shared_counter = %d (expected %d)\n", shared_counter, 2 * N_INCREMENTS);

	return 0;
}
