#pragma once

#include "stm32l4xx.h"

// BB converts a memory mapped register to an array of its bits (one per word)
// e.g. BB(SomeReg)[Bit_position] = 1
#define BB(REG) ((volatile uint32_t*)(PERIPH_BB_BASE + ((uint32_t)&REG - PERIPH_BASE)*32U))
