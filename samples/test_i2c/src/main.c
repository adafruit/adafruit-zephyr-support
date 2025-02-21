/*
 * Copyright (c) 2024 Cirrus Logic, Inc.
 * Copyright (c) 2024 Lucian Copeland for Adafruit Industries
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/haptics.h>
#include <zephyr/drivers/haptics/drv2605.h>
#include <zephyr/sys/util.h>
#include <sys/_stdint.h>
#include <zephyr/drivers/gpio.h>

/* add blinky features, if LED exists */
#if DT_HAS_ALIAS(led0)
#define SLEEP_TIME_MS   1000
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#endif

static struct drv2605_rom_data rom_data = {
	.library = DRV2605_LIBRARY_LRA,
	.brake_time = 0,
	.overdrive_time = 0,
	.sustain_neg_time = 0,
	.sustain_pos_time = 0,
	.trigger = DRV2605_MODE_INTERNAL_TRIGGER,
	.seq_regs[0] = 1,
	.seq_regs[1] = 10 | 0x80,
	.seq_regs[2] = 2,
	.seq_regs[3] = 10 | 0x80,
	.seq_regs[4] = 3,
	.seq_regs[5] = 10 | 0x80,
	.seq_regs[6] = 4,
};

int main(void)
{
	/* Configure haptic device and send a pre-made effect */
	const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(haptic));
	union drv2605_config_data config_data = {};
	int ret;

	if (!dev) {
		return 0;
	}
	if (!device_is_ready(dev)) {
		return 0;
	}
	config_data.rom_data = &rom_data;
	ret = drv2605_haptic_config(dev, DRV2605_HAPTICS_SOURCE_ROM, &config_data);
	if (ret < 0) {
		return 0;
	}
	ret = haptics_start_output(dev);
	if (ret < 0) {
		return 0;
	}

	/* blinky loop to confirm micro is running, in case of hardware failure */

	#if DT_HAS_ALIAS(led0)
	bool led_state = true;
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}

		led_state = !led_state;
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
	#endif
}
