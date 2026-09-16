#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define SW0_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

int main(void)
{
	/* Raw devicetree values: resolved entirely at compile time by the
	 * preprocessor, no driver involved yet. */
	printk("sw0 label: %s\n", DT_PROP(SW0_NODE, label));
	printk("sw0 pin:   %d\n", DT_GPIO_PIN(SW0_NODE, gpios));

	/* Now bind the GPIO driver to that same node and actually read it. */
	if (!gpio_is_ready_dt(&sw0)) {
		printk("sw0 device not ready\n");
		return 0;
	}

	gpio_pin_configure_dt(&sw0, GPIO_INPUT);
	printk("sw0 level: %d\n", gpio_pin_get_dt(&sw0));

	return 0;
}
