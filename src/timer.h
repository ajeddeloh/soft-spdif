#pragma once

#include <stdint.h>

void timer_init();

void timer_disable_spi_clk(uint32_t n_spi_clks);
