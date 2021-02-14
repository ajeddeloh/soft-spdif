#include "stm32l476xx.h"
#include "gpio.h"

#define MODE_INPUT 0
#define MODE_OUTPUT 1
#define MODE_AF 2
#define MODE_ANALOG 3

void gpio_init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODE0_Msk | MODE_OUTPUT;
	GPIOA->OSPEEDR = 0x3 << GPIO_OSPEEDR_OSPEED0_Pos;
}
