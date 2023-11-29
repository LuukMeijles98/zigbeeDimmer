#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>


#define PWM_PERIOD 20000

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

int configLeds(void){
	if (!gpio_is_ready_dt(&led0)) {
		printk("Error: DBG led is not ready");
		return 0;
	}

	if (!pwm_is_ready_dt(&pwm_led0)) {
		printk("Error: PWM device %s is not ready\n",
		       pwm_led0.dev->name);
		return 0;
	}

	int ret;

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	ret = pwm_set_dt(&pwm_led0, PWM_PERIOD, PWM_PERIOD / 2U);
	if (ret) {
		printk("Error %d: failed to set pulse width\n", ret);
		return 0;
	}

	return 1;
}

void toggleLed(struct k_timer *dummy){
		int ret = gpio_pin_toggle_dt(&led0);
		if (ret < 0) {
            printk("Error: Cant toggle debug led\n");
		}
}