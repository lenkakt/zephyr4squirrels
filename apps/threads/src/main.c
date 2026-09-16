/*
 * Copyright (c) 2026 Lenka Koskova Triskova <lenka@koskova.cz>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * threads
 *
 * Two threads run the same entry function, parameterized differently:
 * each just prints its own name, on its own period, forever. Neither
 * thread knows the other exists -- the scheduler interleaves them.
 */

#include <zephyr/kernel.h>

#define STACKSIZE 1024
#define PRIORITY  5

struct thread_config {
  const char *name;
  uint32_t period_ms;
  int repeat_count;
};

static struct thread_config configs[] = {
  { "Thread 1", 1000, 3 },
  { "Thread 2", 700, 3 },
};

static void thread_entry(void *config_ptr, void *unused2, void *unused3)
{
  struct thread_config *cfg = (struct thread_config *)config_ptr;

  ARG_UNUSED(unused2);
  ARG_UNUSED(unused3);

  for (int i = 0; i < cfg->repeat_count; i++) {
    printk("%s (%d/%d)\n", cfg->name, i + 1, cfg->repeat_count);
    k_msleep(cfg->period_ms);
  }
  printk("%s done\n", cfg->name);
}

K_THREAD_DEFINE(thread1_id, STACKSIZE, thread_entry, &configs[0], NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(thread2_id, STACKSIZE, thread_entry, &configs[1], NULL, NULL, PRIORITY, 0, 0);
