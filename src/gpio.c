#include "stm32l476xx.h"
#include "gpio.h"

#define MODE_INPUT 0
#define MODE_OUTPUT 1
#define MODE_AF 2
#define MODE_ANALOG 3

/*
 * Pin mappings
 *
 * PA0: Timer 8 external trigger (spdif in)
 * PA1: Timer 5 Channel 2 output (i2s mclk)
 * PA2: SAI 2 clock in (i2s mclk)
 * PA3: Timer 2 Channel 4 output (spi clk)
 * PA5: Timer 2 external trigger (i2s mclk)
 *
 * PB2: Red LED
 *
 * PE8: Green LED
 * PB10: Debug line
 * PE13: SPI CLK (wired to PA5)
 * PE15: SPI MOSI (spdif in)
 */

#define pinmode_mask(pin, mode) (~GPIO_MODER_MODE##pin##_Msk | (mode << GPIO_MODER_MODE##pin##_Pos))

void gpio_init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOEEN;

	// GPIO A
	GPIOA->MODER &= pinmode_mask(0, MODE_AF)
		& pinmode_mask(1, MODE_AF)
		& pinmode_mask(3, MODE_AF)
		& pinmode_mask(5, MODE_AF);
	GPIOA->AFR[0] = 3
		| (2 << GPIO_AFRL_AFSEL1_Pos)
		| (1 << GPIO_AFRL_AFSEL3_Pos)
		| (2 << GPIO_AFRL_AFSEL5_Pos);
	// high speed PWM out
	GPIOA->OSPEEDR = 0x3 << GPIO_OSPEEDR_OSPEED1_Pos
		| 0x3 << GPIO_OSPEEDR_OSPEED3_Pos
		| 0x3 << GPIO_OSPEEDR_OSPEED5_Pos;

	GPIOB->MODER &= pinmode_mask(2, MODE_OUTPUT);
	GPIOB->OSPEEDR = 0x3 << GPIO_OSPEEDR_OSPEED2_Pos;

	// GPIO E (for SPI)
	GPIOE->MODER &= pinmode_mask(8, MODE_OUTPUT)
		& pinmode_mask(10, MODE_OUTPUT)
		& pinmode_mask(13, MODE_AF)
		& pinmode_mask(15, MODE_AF);
	GPIOE->AFR[1] = (5 << GPIO_AFRH_AFSEL13_Pos)
		| (5 << GPIO_AFRH_AFSEL15_Pos);
	GPIOE->OSPEEDR = 0x3 << GPIO_OSPEEDR_OSPEED10_Pos
		| 0x3 << GPIO_OSPEEDR_OSPEED8_Pos;
}

void gpio_set_leds(int red, int green) {
	GPIOB->BSRR = red ? GPIO_BSRR_BS2 : GPIO_BSRR_BR2;
	GPIOE->BSRR = green ? GPIO_BSRR_BS8 : GPIO_BSRR_BR8;
}

void gpio_debug(int set) {
	GPIOE->BSRR = set ? GPIO_BSRR_BS10 : GPIO_BSRR_BR10;
}
