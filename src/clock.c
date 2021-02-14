#include "stm32l476xx.h"
#include "core_cm4.h"
#include "clock.h"

void clock_init() {
	// Assumes reset state, not safe to call except in reset state
	// Set flash to 4 wait states, with prefetch
	FLASH->ACR |= FLASH_ACR_LATENCY_4WS | FLASH_ACR_PRFTEN;

	// Turn on HSE clock and wait for it to be ready
	RCC->CR |= RCC_CR_HSEON;
	while (! (RCC->CR & RCC_CR_HSERDY)) {}

	// Configure, turn on, then wait for the PLL
	// M=1, N=20, P/Q are unused, R=2
	// Output should be 80MHz
	RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSE | (20 << RCC_PLLCFGR_PLLN_Pos);
	RCC->CR |= RCC_CR_PLLON;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
	while (!(RCC->CR & RCC_CR_PLLRDY)) {}
	
	RCC->CFGR |= RCC_CFGR_SW_PLL;
}
