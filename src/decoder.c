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

static void change_state(DecoderState new);
static uint32_t rev(uint32_t in);
static uint32_t rbit(uint32_t in);
static int32_t get_sync_offset(uint32_t *words);
static int32_t check_sync(const volatile uint8_t *frame);
static void unpack_frame_to_words(const volatile uint8_t *frame, uint32_t *words);
__attribute__((noinline)) static int32_t unpack_data(const uint32_t *frame, int16_t *ret);

/*
 * decoder_handle_frame processes the captured frame, either syncing the spi device to the
 * start of the frame or decoding the data if the stream is sycned.
 */
void decoder_handle_frame(volatile uint8_t *frame) {
	if (check_sync(frame)) {
		change_state(SYNCED);
	} else if (state == SYNCED) {
		change_state(UNSYNCED);
	}
	int32_t sync_offset;
	uint32_t frame_words[4];
	int16_t data[2] = {0, 0};
	unpack_frame_to_words(frame, frame_words);

	switch (state) {
		case UNSYNCED:
			// todo unlight synced led
			sync_offset = get_sync_offset(frame_words);
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
			unpack_data(frame_words, data);
			// todo, decode biphase mark and copy sample into i2s buffer
			break;
	}
	frame[0] = (uint8_t) data[0];

}

/*
 * change_state modifies the decoder state to the new one and sets the LEDs to indicate
 * the new state
 */
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

// rev is a wrapper around the rev instruction. Swaps endianess.
static uint32_t rev(uint32_t in) {
	uint32_t ret;
	__asm__("rev %0, %1"
			: "=r" (ret)
			: "r" (in));
	return ret;
}

/*
 * rbit is a wrapper around the rbit instruction. Reverses the order of bits in the word.
 */
static uint32_t rbit(uint32_t in) {
	uint32_t ret;
	__asm__("rbit %0, %1"
			: "=r" (ret)
			: "r" (in));
	return ret;
}

/*
 * unpack_frame_to_words copies the frame's data from frame to words and fixes the endianess
 * as well.
 */
static void unpack_frame_to_words(const volatile uint8_t *frame, uint32_t *words) {
	volatile uint32_t *frame_as_words = (uint32_t *)frame;
	for (int i = 0; i < 4; i++) {
		words[i] = rev(frame_as_words[i]);
	}
}

/*
 * get_sync_offset returns the offset in spdif clock cycles for where the start of the frame
 * is in the currently captured block
 */
static int32_t get_sync_offset(uint32_t *words) {
	// Need to do bit operations but the data is DMA'd in the wrong endianess for that
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

/*
 * check_sync returns if the block of data in frame is actually aligned to the start of a spdif
 * frame.
 * Returns true if the frame is synced, false otherwise
 */
static int32_t check_sync(const volatile uint8_t *frame) {
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

/*
 * unpack_data unpacks the frame into two shorts with the actual values transmitted. Marked
 * as noinline purely for debugging purposes.
 * */
__attribute__((noinline)) static int32_t unpack_data(const uint32_t *frame, int16_t *ret) {
	uint32_t words[2] = {(frame[0] << 24) | (frame[1] >> 8), 
		 (frame[2] << 24) | (frame[3] >> 8)};

	int32_t val = 0;
	for (int i = 0; i < 2; i++) {
		uint32_t tmp = words[i] ^ (words[i] >> 1);
		// now every even bit is correct, need to smoosh down to 16 bits
		for (int bit = 0; bit < 16; bit ++) {
			val |= (int32_t) (((tmp >> (bit*2)) & 0x1) << bit);
		}
		ret[i] = rbit(val) >> 16;
	}
	return  0;
}
