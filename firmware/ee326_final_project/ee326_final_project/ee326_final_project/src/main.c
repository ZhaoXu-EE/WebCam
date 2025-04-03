/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include "conf_board.h"
#include "conf_clock.h"
#include "wifi.h"
#include "camera.h"
#include "ov2640.h"
#include "timer_interface.h"

char* buff[50];

int main(void) {
	// Initialize system clock, watchdog timer, and I/O ports
	sysclk_init();
	wdt_disable(WDT);
	ioport_init();
	board_init();

	// Configure peripherals and timers
	configure_tc();
	configure_usart();
	configure_spi();
	configure_wifi_comm_pin();
	configure_wifi_provision_pin();

	// Initialize and configure the camera
	init_camera();
	configure_camera();

	// Reset WiFi module
	ioport_set_pin_level(WIFI_RESET_MASK, false);
	delay_ms(100);
	ioport_set_pin_level(WIFI_RESET_MASK, true);
	delay_ms(500);

	// Set SPI baud rate for communication
	sprintf(buff, "set spi_baud %d", SPI_BAUDRATE);
	write_wifi_command(buff, 2);

	// Configure ESP32 indicator LEDs
	sprintf(buff, "set wlan_gpio %d", ESP_NET_LED);
	write_wifi_command(buff, 2);
	sprintf(buff, "set websocket_gpio %d", ESP_CLIENT_LED);
	write_wifi_command(buff, 2);
	sprintf(buff, "set ap_gpio %d", ESP_PROV_LED);
	write_wifi_command(buff, 2);

	// Configure WiFi control pins
	sprintf(buff, "set comm_gpio %d", ESP_COMM_GPIO);
	write_wifi_command(buff, 2);
	sprintf(buff, "set net_gpio %d", ESP_NET_GPIO);
	write_wifi_command(buff, 2);
	sprintf(buff, "set clients_gpio %d", ESP_CLIENT_GPIO);
	write_wifi_command(buff, 2);

	reading_wifi_flag = false;
	provisioning_flag = false;

	// Wait for network connection acknowledgment
	while (!ioport_get_pin_level(WIFI_NET_MASK)) {
		if (provisioning_flag) {
			write_wifi_command("provision", 1);
			provisioning_flag = false;
			write_wifi_command("get mac", 1);
		}
	}

	delay_ms(100);
	write_wifi_command("test", 10);
	delay_ms(1000);

	// Reinitialize WiFi module if not connected
	while (ioport_get_pin_level(WIFI_NET_MASK) && ioport_get_pin_level(WIFI_CLIENT_PIN_MASK)) {
		ioport_set_pin_level(WIFI_RESET_MASK, false);
		delay_ms(100);
		ioport_set_pin_level(WIFI_RESET_MASK, true);
		delay_ms(5000);
		write_wifi_command("test", 10);
		delay_ms(1000);
	}

	// Main loop for image capture and transmission
	while (true) {
		if (provisioning_flag) {
			write_wifi_command("provision", 1);
			provisioning_flag = false;
			} else if (ioport_get_pin_level(WIFI_CLIENT_PIN_MASK)) {
			start_capture();
			if (len_success) {
				write_image_to_web();
			}
		}
	}
}
