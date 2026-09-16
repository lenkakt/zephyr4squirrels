#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

static int cmd_squirrel(const struct shell *sh, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  shell_print(sh, "A squirrel appears and steals your bandwidth.");
  return 0;
}

SHELL_CMD_REGISTER(squirrel, NULL, "Summon a squirrel.", cmd_squirrel);
int main(void)
{
	printk("Hello, squirrels! Zephyr is alive.\n");

	return 0;
}
