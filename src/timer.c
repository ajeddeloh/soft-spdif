#include "stm32l476xx.h"

#include "timer.h"
#include "bit_band.h"
#include "gpio.h"

#define TIM_SMCR_TS_EXTTRIG (TIM_SMCR_TS_0 | TIM_SMCR_TS_1 | TIM_SMCR_TS_2)
#define TIM_SMCR_TS_ITR3 (TIM_SMCR_TS_0 | TIM_SMCR_TS_1)
#define TIM_CCMR_OCM_PWM 0x6

#define SPI_CLK_PERIOD 14 // 80MHz / (44.1kHz * 32bits/frame * 2frames * 2wire bits/real bit)
#define MCLK_PERIOD (SPI_CLK_PERIOD/2)

#define OPM_DELAY 10 // since the timers cant immedately compare

// TIM8 is a dummy timer that just relays its TRGI to the mclk timer
#define sync_timer TIM8

// TIM4 is the i2s master clock, at 2x the freq of the spi clock (~11.2MHz)
#define mclk_timer TIM5

// TIM5 is the spi clock, using TIM4 as its SPI clock
#define spi_timer TIM2

// TIM1 is the bit synchronization timer, default on unless we need to shift the bits
#define shift_timer TIM4

void timer_init() {
	// Turn on clock for TIM1 and TIM8
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM5EN | RCC_APB1ENR1_TIM4EN;

	// sync timer
	// set output CH1 to PWM mode and enable it
	sync_timer->CCMR1 = TIM_CCMR_OCM_PWM << TIM_CCMR1_OC1M_Pos;
	sync_timer->CCER = TIM_CCER_CC1E;
	// set up period/duty cycle
	sync_timer->ARR = MCLK_PERIOD - 1;
	sync_timer->CCR1 = MCLK_PERIOD / 2;
	// Set CH1 as the TRGO
	sync_timer->CR2 = TIM_CR2_MMS_2;
	// Reset and trigger on external rising edges from spdif
	sync_timer->SMCR = TIM_SMCR_SMS_3 | TIM_SMCR_TS_EXTTRIG | TIM_SMCR_MSM;

	// mclk
	// set output CH2 to PWM mode and enable it
	mclk_timer->CCMR1 = TIM_CCMR_OCM_PWM << TIM_CCMR1_OC2M_Pos;
	mclk_timer->CCER = TIM_CCER_CC2E;
	// set the clock period and duty cycle
	mclk_timer->ARR = MCLK_PERIOD * 2; // doesn't really matter since it will be reset
	mclk_timer->CCR2 = MCLK_PERIOD / 2;
	// Reset and trigger on ITR3 (which is the sync timer)
	mclk_timer->SMCR = TIM_SMCR_SMS_3 | TIM_SMCR_TS_ITR3;

	// spi clk
	// set output CH4 to PWM mode 2 and enable it
	spi_timer->CCMR2 = (7 << TIM_CCMR2_OC4M_Pos);
	spi_timer->CCER = TIM_CCER_CC4E;
	// timings for dividing the input clock by 2
	spi_timer->CCR4 = 1;
	spi_timer->ARR = 1;
	// Use the TIM8 output as the external clock
	spi_timer->SMCR = TIM_SMCR_ECE // ext clk mode 2 (from mclk over hooked up to ETR (gpio PA5))
		| TIM_SMCR_SMS_0 | TIM_SMCR_SMS_2 // gated mode (on shift timer)
		| TIM_SMCR_TS_ITR3; // TRGI is ITR3 (shift timer)
	spi_timer->CR1 = TIM_CR1_CEN;


	// shift
	// set output to CH1  and enable it
	shift_timer->CCMR1 = TIM_CCMR_OCM_PWM << TIM_CCMR1_OC1M_Pos;
	shift_timer->CCER = TIM_CCER_CC1E;
	// delay before disabling the clock
	shift_timer->CCR1 = OPM_DELAY;
	// set TRGO (which gates the spi clk timer) to be CH1 output
	shift_timer->CR2 = TIM_CR2_MMS_2;
	shift_timer->SMCR = TIM_SMCR_TS_ITR3 // TRGI is TIM8 update
		| TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_2; // clock on TRGI
	// turn on one pulse mode
	shift_timer->CR1 = TIM_CR1_OPM;
}

void timer_disable_spi_clk(uint32_t n_spi_clks) {
	// *2 since the input is from mclk not spi clk
	shift_timer->ARR = OPM_DELAY + (n_spi_clks * 2) - 1;
	BB(shift_timer->CR1)[TIM_CR1_CEN_Pos] = 1;
}
