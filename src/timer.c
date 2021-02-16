#include "stm32l476xx.h"

#include "timer.h"

#define SMCR_EXT_TRIG (0x7 << TIM_SMCR_TS_Pos)
#define CCMR_PWM 6

#define SPI_CLK_PERIOD 13 // 80MHz / (48kHz * 32bits/frame * 2frames * 2wire bits/real bit)

void timer_init() {
	// Turn on clock
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;

	// Turn on the timer
	TIM8->CCMR1 |= CCMR_PWM << TIM_CCMR1_OC1M_Pos;
	TIM8->CCER |= TIM_CCER_CC1NE | TIM_CCER_CC1E;
	TIM8->ARR = SPI_CLK_PERIOD - 1;
	TIM8->CCR1 = SPI_CLK_PERIOD / 2;
	TIM8->BDTR |= TIM_BDTR_MOE;

	// Set it active
	TIM8->SMCR |= TIM_SMCR_SMS_3 | SMCR_EXT_TRIG;
}
