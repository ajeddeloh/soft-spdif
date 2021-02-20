#include <stdint.h>

#include "decoder.h"
#include "timer.h"
#include "gpio.h"

#define B_MAGIC 0xE8
#define M_MAGIC 0xE2
#define W_MAGIC 0xE4
#define IB_MAGIC ((uint8_t)~B_MAGIC)
#define IM_MAGIC ((uint8_t)~M_MAGIC)
#define IW_MAGIC ((uint8_t)~W_MAGIC)

typedef enum DecoderState {
	UNSYNCED,
	SYNCING,
	SYNCED,
} DecoderState;

static DecoderState state = UNSYNCED;

static int32_t get_sync_offset(volatile uint8_t *frame);
static int32_t check_sync(volatile uint8_t *frame);

static void change_state(DecoderState new) {
	state = new;
	switch (state) {
		case UNSYNCED:
			gpio_set_leds(0, 0);
			break;
		case SYNCING:
			gpio_set_leds(1, 1);
			break;
		case SYNCED:
			gpio_set_leds(0, 1);
			break;
		default:
			gpio_set_leds(1, 0);
			break;
	}
}

void decoder_handle_frame(volatile uint8_t *frame) {
	if (check_sync(frame)) {
		change_state(SYNCED);
	} else if (state == SYNCED) {
		change_state(UNSYNCED);
	}
	int32_t sync_offset;
	switch (state) {
		case UNSYNCED:
			// todo unlight synced led
			sync_offset = get_sync_offset(frame);
			if (sync_offset == -1) {
				// todo light error led
			} else {
				timer_disable_spi_clk((uint32_t)sync_offset);
				change_state(SYNCING);
			}
			break;
		case SYNCING:
			change_state(UNSYNCED);
			break;
		case SYNCED:
			// todo, decode biphase mark and copy sample into i2s buffer
			break;
	}

}

int32_t get_sync_offset(volatile uint8_t *frame) {
	if (check_sync(frame)) {
		return 0;
	}
	// Need to do bit operations but the data is DMA'd in the wrong endianess for that
	volatile uint32_t *frame_as_words = (uint32_t *)frame;
	uint32_t words[3];
	for (int i = 0; i < 3; i++) {
		// swap endianess, using assembly
		uint32_t tmp_word;
		__asm__("rev %0, %1"
				: "=r" (tmp_word)
				: "r" (frame_as_words[i])
				: "memory");
		words[i] = tmp_word;
	}
	
	for(int i = 1; i < 72; i++) {
		words[0] = (words[0] << 1) | (words[1] >> 31);
		uint8_t msb = words[0] >> 24;
		switch (msb) {
			case B_MAGIC:
			case IB_MAGIC: 
			case M_MAGIC: 
			case IM_MAGIC:
				return i;
			case W_MAGIC:
			case IW_MAGIC:
				return i + 64;
		}
		words[1] = (words[1] << 1) | (words[2] >> 31);
		words[2] = words[2] << 1;
	}
	return -1;
}

int32_t check_sync(volatile uint8_t *frame) {
	switch (frame[0]) {
		case B_MAGIC:
		case IB_MAGIC:
		case M_MAGIC:
		case IM_MAGIC:
			return (frame[8] == W_MAGIC || frame[8] == IW_MAGIC);
		default:
			return 0;
	}
}
