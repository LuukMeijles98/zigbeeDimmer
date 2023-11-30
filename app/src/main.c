/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <app_version.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/net/buf.h>
#include <zephyr/net/ieee802154_radio.h>
#include <zephyr/net/ieee802154_mgmt.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/socket.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define TEST_PAYLOAD "test"

/* led functions */
extern int configLeds(void);
extern void toggleLed(struct k_timer *dummy);
extern int setPwm(uint32_t pwm_ds);

K_TIMER_DEFINE(dbg_led_timer, toggleLed, NULL);

#if IS_ENABLED(CONFIG_NET_TC_THREAD_COOPERATIVE)
#define THREAD_PRIORITY K_PRIO_COOP(CONFIG_NUM_COOP_PRIORITIES -1)
#else
#define THREAD_PRIORITY K_PRIO_PREEMPT(8)
#endif

/*
 *  Stack for the tx thread
 */
static K_THREAD_STACK_DEFINE(tx_stack, 1024);
static struct k_thread tx_thread_data;

static void tx_thread(void){
	int ret;

	struct net_if *iface = net_if_get_ieee802154();

	while(1){
		struct net_pkt *pkt;

		pkt = net_pkt_alloc_with_buffer(iface, sizeof(TEST_PAYLOAD), AF_UNSPEC, 0, K_NO_WAIT);

		if(pkt == NULL){
			printk("Unable to allocate packet");
		}

		ret = net_pkt_write(pkt, TEST_PAYLOAD, sizeof(TEST_PAYLOAD));
		if(ret < 0){
			printk("Unable to write! Err: %i\n", ret);
		}

		ret = net_send_data(pkt);
		if(ret < 0){
			printk("Unable to send! Err %i\n", ret);
		}

		net_pkt_unref(pkt);

		
		k_msleep(1000);
	}
}

int main(void){
	int ret;
	struct zsock_pollfd fds;
	struct sockaddr_ll socket_ssl = {0};

	printk("Starting led dimmer\n");

	if(!configLeds()){
		printk("An error occured while configuring the LEDs\n");
		return 0;
	}

	struct net_if *iface = net_if_get_ieee802154();
	if(iface == NULL){
		printk("Unable to get 802.15.4 interface.");
	}
	
	k_timer_start(&dbg_led_timer, K_SECONDS(1), K_SECONDS(1));

	/* enable recieving */
	fds.fd = zsock_socket(AF_PACKET, SOCK_DGRAM, ETH_P_IEEE802154);
	if(fds.fd < 0){
		printk("Failed to create RAW socket. Err %i\n", errno);
		return 0;
	}

	fds.events = ZSOCK_POLLIN;

	socket_ssl.sll_ifindex = net_if_get_by_iface(iface);
	socket_ssl.sll_family = AF_PACKET;
	socket_ssl.sll_protocol = ETH_P_IEEE802154;

	if(zsock_bind(fds.fd, (const struct sockaddr *)&socket_ssl, sizeof(struct sockaddr_ll))){
		printk("Failed to bind packet socket. Err %i\n", errno);
		return 0;
	}

	/* Start TX thread */
	k_thread_create(&tx_thread_data, tx_stack, K_THREAD_STACK_SIZEOF(tx_stack), (k_thread_entry_t)tx_thread, NULL, NULL, NULL, THREAD_PRIORITY, 0, K_NO_WAIT);

	while (1) {
		uint8_t buf[256];

		ret = zsock_poll(&fds, 1, 1000);
		if(ret == 0){
			continue;
		}
		else if(ret < 0){
			printk("Unable to poll radio! Err %i\n", ret);
			break;
		}

		ret = zsock_recv(fds.fd, buf, sizeof(buf), 0);
		if(ret > 0){
			printk("%i bytes recieved!\n", ret);
			printk("buff = %s\n", buf);
		}
		else{
			printk("Unable to recieve data from radio! Err %i\n", ret);
		}
	}

	return 0;
}

