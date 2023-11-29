/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <app_version.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

/* led functions */
extern int configLeds(void);
extern void toggleLed(struct k_timer *dummy);
K_TIMER_DEFINE(dbg_led_timer, toggleLed, NULL);

int main(void){
	printk("Starting led dimmer\n");

	if(!configLeds()){
		printk("An error occured while configuring the LEDs\n");
	}
	
	k_timer_start(&dbg_led_timer, K_SECONDS(1), K_SECONDS(1));

	while (1) {
		k_msleep(1000);
	}

	return 0;
}

