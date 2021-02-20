#include "stm32l476xx.h"

#include "spi.h"
#include "decoder.h"

// each subframe is 64 wire bits, store two at a time
static volatile uint8_t spi_buf[FRAME_SIZE_BYTES * 2];

#define SPI_CR2_DS_8BIT (SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2)

void spi_init() {
	// enable the clock for SPI1 and DMA
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	NVIC->ISER[0] = 1 << DMA1_Channel2_IRQn;

	// Turn on DMA
	DMA1_Channel2->CNDTR = sizeof(spi_buf);
	DMA1_Channel2->CPAR = (uint32_t)&(SPI1->DR);
	DMA1_Channel2->CMAR = (uint32_t)&spi_buf;
	DMA1_CSELR->CSELR |= 1 << DMA_CSELR_C2S_Pos;
	DMA1_Channel2->CCR = DMA_CCR_MINC | DMA_CCR_CIRC
		| DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_TEIE | DMA_CCR_EN;

	// Turn on SPI
	SPI1->CR2 = SPI_CR2_DS_8BIT | SPI_CR2_RXDMAEN;
	SPI1->CR1 = SPI_CR1_RXONLY | SPI_CR1_CPHA | SPI_CR1_SSM | SPI_CR1_SPE;
}

void DMA1_Channel2_IRQHandler() {
	uint32_t isr = DMA1->ISR;
	DMA1->IFCR = 0xf0;
	if (isr & DMA_ISR_TEIF2) {
		while (1) {} // todo handle error
	}
	if ((isr & DMA_ISR_TCIF2) && (isr & DMA_ISR_HTIF2)) {
		while(1) {} // overrun
	}
	if (isr & DMA_ISR_HTIF2) {
		decoder_handle_frame(spi_buf);
	}
	if (isr & DMA_ISR_TCIF2) {
		decoder_handle_frame(spi_buf + sizeof(spi_buf) / 2);
	}
}
