#include "clock.h"
#include "gpio.h"

#include "stm32l476xx.h"

int main() {
	clock_init();
	gpio_init();

	while(1) {
		GPIOA->ODR = 1;
		GPIOA->ODR = 0;
	}
	return 0;
}
