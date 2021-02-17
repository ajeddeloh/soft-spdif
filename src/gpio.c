#include "stm32l476xx.h"
#include "gpio.h"

#define MODE_INPUT 0
#define MODE_OUTPUT 1
#define MODE_AF 2
#define MODE_ANALOG 3

void gpio_init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOEEN;

	// GPIO A
	GPIOA->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE5_Msk)
		| MODE_AF                            // TIM8 Ext trig
		| (MODE_AF << GPIO_MODER_MODE1_Pos)  // TIM5 CH2 out (NSS controller)
		| (MODE_AF << GPIO_MODER_MODE5_Pos); // TIM8 CH1 out (clock out)
	GPIOA->AFR[0] = 3
		| (2 << GPIO_AFRL_AFSEL1_Pos)
		| (3 << GPIO_AFRL_AFSEL5_Pos);

	// high speed PWM out
	GPIOA->OSPEEDR = 0x3 << GPIO_OSPEEDR_OSPEED1_Pos
		| 0x3 << GPIO_OSPEEDR_OSPEED5_Pos;

	// GPIO E (for SPI)
	GPIOE->MODER &= ~(GPIO_MODER_MODE12_Msk | GPIO_MODER_MODE13_Msk | GPIO_MODER_MODE15_Msk)
		| (MODE_AF << GPIO_MODER_MODE12_Pos)  // NSS
		| (MODE_AF << GPIO_MODER_MODE13_Pos)  // SCLK
		| (MODE_AF << GPIO_MODER_MODE15_Pos); // MOSI

	GPIOE->AFR[1] = (5 << GPIO_AFRH_AFSEL12_Pos)
		| (5 << GPIO_AFRH_AFSEL13_Pos)
		| (5 << GPIO_AFRH_AFSEL15_Pos);
}
