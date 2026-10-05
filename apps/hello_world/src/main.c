#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

static int parse_color(const char *name, enum shell_vt100_color *out);

static int cmd_squirrel(const struct shell *sh, size_t argc, char **argv)
{
  enum shell_vt100_color color = SHELL_VT100_COLOR_DEFAULT;

  if (argc > 1 && parse_color(argv[1], &color) != 0) {
    shell_error(sh, "Unknown color '%s'", argv[1]);
    return -EINVAL;
  }

  if (color == SHELL_VT100_COLOR_DEFAULT) {
    shell_print(sh, "A squirrel appears and steals your bandwidth...\n");
  } else {
    shell_print(sh, "A %s squirrel appears and steals your bandwidth...\n", argv[1]);
  }

  // Squirrel ASCII art from https://ascii.co.uk/art/squirrel
  shell_fprintf(sh, color, "          )\" .\n");
  shell_fprintf(sh, color, "         /    \\      (\\-./");
  if (argc > 2) shell_fprintf(sh, color, "     %s", argv[2]);
  shell_print(sh, "");
  shell_fprintf(sh, color, "        /     |    _/ o. \\");
  if (argc > 2) shell_fprintf(sh, color, "    /");
  shell_print(sh, "");
  shell_fprintf(sh, color, "       |      | .-\"      y)-\n");
  shell_fprintf(sh, color, "       |      |/       _/ \\\n");
  shell_fprintf(sh, color, "       \\     /j   _\".\\(@)\n");
  shell_fprintf(sh, color, "        \\   ( |    `.''  )\n");
  shell_fprintf(sh, color, "         \\  _`-     |   /\n");
  shell_fprintf(sh, color, "           \"  `-._  <_ (\n");
  shell_fprintf(sh, color, "                  `-.,),)\n");

  return 0;
}

SHELL_CMD_ARG_REGISTER(
  squirrel, NULL,
	"Summon a squirrel.\n Usage: squirrel [color] [message]\n Colors: default, red, green, yellow, blue, magenta, cyan, white, black",
  cmd_squirrel,
  1,  /* number of mandatory arguments */
  2   /* number of optional arguments */
);
int main(void)
{
	printk("Hello, squirrels! Zephyr is alive.\n");

	return 0;
}


static int parse_color(const char *name, enum shell_vt100_color *out)
{
  struct color_entry {
    const char *name;
    enum shell_vt100_color color;
  };

  static const struct color_entry colors[] = {
    {"default", SHELL_VT100_COLOR_DEFAULT}, {"red", SHELL_VT100_COLOR_RED}, {"green", SHELL_VT100_COLOR_GREEN},
    {"yellow", SHELL_VT100_COLOR_YELLOW}, {"blue", SHELL_VT100_COLOR_BLUE}, {"magenta", SHELL_VT100_COLOR_MAGENTA},
    {"cyan", SHELL_VT100_COLOR_CYAN}, {"white", SHELL_VT100_COLOR_WHITE}, {"black", SHELL_VT100_COLOR_BLACK}
  };

	for (size_t i = 0; i < ARRAY_SIZE(colors); i++) {
		if (strcmp(name, colors[i].name) == 0) {
			*out = colors[i].color;
			return 0;
		}
	}
 
	return -EINVAL;
}
