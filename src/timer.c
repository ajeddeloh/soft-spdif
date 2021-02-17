#include "stm32l476xx.h"

#include "timer.h"
#include "bit_band.h"

#define TIM_SMCR_TS_EXTTRIG (TIM_SMCR_TS_0 | TIM_SMCR_TS_1 | TIM_SMCR_TS_2)
#define TIM_CCMR_OCM_PWM 0x6

#define SPI_CLK_PERIOD 13 // 80MHz / (48kHz * 32bits/frame * 2frames * 2wire bits/real bit)

#define NSS_DELAY 10 // since the timers cant immedately compare

// TIM8 is the spi clock
#define spi_timer TIM8
// TIM5 is hooked up to SPI1's NSS pin to sync the SPI stream
#define nss_timer TIM5

void timer_init() {
	// Turn on clock for TIM1 and TIM8
	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;

	// Turn on the timer
	// set output CH1 to PWM mode
	spi_timer->CCMR1 = TIM_CCMR_OCM_PWM << TIM_CCMR1_OC1M_Pos;
	// enable CH1N
	spi_timer->CCER = TIM_CCER_CC1NE;
	// set the clock period and duty cycle
	spi_timer->ARR = SPI_CLK_PERIOD - 1;
	spi_timer->CCR1 = SPI_CLK_PERIOD / 2;
	// enable outputs
	spi_timer->BDTR = TIM_BDTR_MOE;
	// set up TRGO to use CH1 so it can be a clock to nss_timer
	spi_timer->CR2 = TIM_CR2_MMS_2;

	// Set it active, started and reset by external trigger
	spi_timer->SMCR = TIM_SMCR_SMS_3 | TIM_SMCR_TS_EXTTRIG;

	// Set up everything in TIM1 except ARR and enabling it
	// Set the compare value to >0 so we don't immediately turn on the output after
	// switching to pwm mode
	nss_timer->CCR2 = NSS_DELAY;
	// set output CH2 to PWM mode 2
	nss_timer->CCMR1 = (7 << TIM_CCMR1_OC2M_Pos);
	// enable CH2
	nss_timer->CCER = TIM_CCER_CC2E;
	// Use the TIM8 output as the external clock
	nss_timer->SMCR = TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1 | TIM_SMCR_SMS_2 // ext clock mode 1
		| TIM_SMCR_TS_0 | TIM_SMCR_TS_1; // trigger on tim 8
	// one pulse mode
	nss_timer->CR1 = TIM_CR1_OPM;

}

void timer_pulse_spi_nss(uint32_t n_spi_clks) {
	nss_timer->ARR = n_spi_clks + NSS_DELAY + 1;
	BB(nss_timer->CR1)[TIM_CR1_CEN_Pos] = 1;//enable it
}
