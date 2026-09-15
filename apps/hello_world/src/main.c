/*
 * Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * hello_world
 *
 * The smallest possible Zephyr application. It boots the kernel,
 * runs main(), prints one line, and returns.
 */

#include <zephyr/kernel.h>

int main(void)
{
	printk("Hello, squirrels! Zephyr is alive.\n");

	return 0;
}
