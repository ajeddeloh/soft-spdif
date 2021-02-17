#pragma once

#include <stdint.h>

void timer_init();

void timer_pulse_spi_nss(uint32_t n_spi_clks);
