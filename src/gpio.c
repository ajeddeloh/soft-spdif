#include "stm32l476xx.h"
#include "gpio.h"

#define MODE_INPUT 0
#define MODE_OUTPUT 1
#define MODE_AF 2
#define MODE_ANALOG 3

void gpio_init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	GPIOA->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE5_Msk)
		| MODE_AF | (MODE_AF << GPIO_MODER_MODE5_Pos);
	GPIOA->AFR[0] = 3                      // TIM8 ETR
		| (3 << GPIO_AFRL_AFSEL5_Pos); // TIM8 CH1N

	GPIOA->OSPEEDR = 0x3 << GPIO_OSPEEDR_OSPEED5_Pos; // high speed PWM out
}
