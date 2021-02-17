#include "stm32l476xx.h"
#include "spi.h"

static uint16_t spi_buf[32];

void spi_init() {
	// enable the clock for SPI1 and DMA
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	// Turn on DMA
	DMA1_Channel2->CNDTR = 32;
	DMA1_Channel2->CPAR = (uint32_t)&(SPI1->DR);
	DMA1_Channel2->CMAR = (uint32_t)&spi_buf;
	DMA1_CSELR->CSELR |= 1 << DMA_CSELR_C2S_Pos;
	DMA1_Channel2->CCR = DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 // 16 bit mem/periph size
		| DMA_CCR_MINC | DMA_CCR_EN;

	// Turn on SPI
	SPI1->CR2 |= (0xF << SPI_CR2_DS_Pos) | SPI_CR2_RXDMAEN;
	SPI1->CR1 |= SPI_CR1_RXONLY | SPI_CR1_CPHA | SPI_CR1_SPE;
}
