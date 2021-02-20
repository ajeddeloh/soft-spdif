#pragma once

#include <stdint.h>

#define FRAME_SIZE_BYTES 16

void decoder_handle_frame(volatile uint8_t *frame);
