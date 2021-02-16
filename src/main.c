#include "clock.h"
#include "gpio.h"
#include "timer.h"
#include "spi.h"

#include "stm32l476xx.h"

int main() {
	clock_init();
	gpio_init();
	timer_init();
	spi_init();

	while(1) {
	}
	return 0;
}
