#include <avr/io.h>
#include <stdint.h>
#include <Arduino.h>
#include <HardwareSerial.h>
#include "MBI5030.h"

uint16_t pwm_buffer[PWM_BUFFER_SIZE] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF,
	0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static inline void pulse_spi_clk(void);
static inline void spi_clk_high(void);
static inline void spi_clk_low(void);
static inline void spi_latch_high(void);
static inline void spi_latch_low(void);
static inline void spi_out_high(void);
static inline void spi_out_low(void);

void spi_setup(void)
{
	SPI_OUT_DIR |= _BV(SPI_OUT_pin);	// set as output
	spi_out_low();

	SPI_CLK_DIR |= _BV(SPI_CLK_pin);	// set as output
	spi_clk_low();

	SPI_LATCH_DIR |= _BV(SPI_LATCH_pin);	// set as output
	spi_latch_low();

	SPI_IN_DIR &= ~_BV(SPI_IN_pin);	// set as input
}

void update_pwm(uint16_t * pwm_data)
{
	uint8_t data_word;
	uint8_t data_word_bit;
	uint16_t pwm_data_tmp;

	// send the first 15 words with "data-latch"
	// the input shift register is 16bit wide
	// it requires a "data-latch" to move the data to
	// the chip-internal buffers

	for (data_word = 0; data_word <= (PWM_BUFFER_SIZE - 2); data_word++) {

		pwm_data_tmp = pwm_data[data_word];

		for (data_word_bit = 0; data_word_bit <= 14; data_word_bit++) {
			// set or clear data - MSB first !
			if (pwm_data_tmp & _BV(15)) {
				spi_out_high();
			} else {
				spi_out_low();
			}
			// pulse spi clock and shift temporary data by 1 to the left
			pulse_spi_clk();
			pwm_data_tmp <<= 1;
		}

		spi_latch_high();	// "data-latch" START

		if (pwm_data_tmp & _BV(15)) {
			spi_out_high();
		} else {
			spi_out_low();
		}
		pulse_spi_clk();

		spi_latch_low();	// "data-latch" END
	}

	// send the last word with "global-latch" 
	// to transfer the last word and make the data "live"

	pwm_data_tmp = pwm_data[15];

	for (data_word_bit = 0; data_word_bit <= 12; data_word_bit++) {
		// set or clear data
		if (pwm_data_tmp & _BV(15)) {
			spi_out_high();
		} else {
			spi_out_low();
		}
		pulse_spi_clk();
		pwm_data_tmp <<= 1;
	}

	spi_latch_high();	// "global-latch" START

	if (pwm_data_tmp & _BV(15)) {
		spi_out_high();
	} else {
		spi_out_low();
	}
	pulse_spi_clk();
	pwm_data_tmp <<= 1;

	if (pwm_data_tmp & _BV(15)) {
		spi_out_high();
	} else {
		spi_out_low();
	}
	pulse_spi_clk();
	pwm_data_tmp <<= 1;

	if (pwm_data_tmp & _BV(15)) {
		spi_out_high();
	} else {
		spi_out_low();
	}
	pulse_spi_clk();

	spi_latch_low();	// "global-latch" END
}

inline void pulse_spi_clk(void)
{
	SPI_CLK_PORT |= _BV(SPI_CLK_pin);
	SPI_CLK_PORT &= ~_BV(SPI_CLK_pin);
}

inline void spi_clk_high(void)
{
	SPI_CLK_PORT |= _BV(SPI_CLK_pin);
}

inline void spi_clk_low(void)
{
	SPI_CLK_PORT &= ~_BV(SPI_CLK_pin);
}

inline void spi_latch_high(void)
{
	SPI_LATCH_PORT |= _BV(SPI_LATCH_pin);
}

inline void spi_latch_low(void)
{
	SPI_LATCH_PORT &= ~_BV(SPI_LATCH_pin);
}

inline void spi_out_high(void)
{
	SPI_OUT_PORT |= _BV(SPI_OUT_pin);
}

inline void spi_out_low(void)
{
	SPI_OUT_PORT &= ~_BV(SPI_OUT_pin);
}

void enable_error_detection(void)
{
	spi_latch_high();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	spi_clk_high();
	spi_latch_low();
	spi_clk_low();
	delayMicroseconds(64);	// some time to stabilize readings
}

void prepare_error_report(void)
{
	spi_latch_high();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	spi_clk_high();
	spi_latch_low();
	spi_clk_low();
}

uint16_t read_register(void)
{
	uint16_t register_status = 0;
	uint8_t register_status_bit;

	// read bits 0-14
	for (register_status_bit = 0; register_status_bit <= 14;
	     register_status_bit++) {
		if (SPI_IN_PIN & _BV(SPI_IN_pin)) {
			register_status |= _BV(0);
		} else {
			// already full with zeros
		}
		pulse_spi_clk();
		register_status <<= 1;
	}

	// read bit 15
	if (SPI_IN_PIN & _BV(SPI_IN_pin)) {
		register_status |= _BV(0);
	} else {
		// already full with zeros
	}

	return register_status;
}

void prepare_config_read(void)
{
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	spi_latch_high();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	pulse_spi_clk();
	spi_clk_high();
	spi_latch_low();
	spi_clk_low();
}

void write_config(uint16_t config_mask, uint8_t current_gain)
{
	uint16_t current_gain_mask = (((uint16_t) (current_gain)) << 2);
	uint16_t config_data = (0x0000 | config_mask | current_gain_mask);
	uint8_t config_data_bit;

	Serial.begin(9600);
	Serial.print("current-gain-mask: ");
	Serial.println(current_gain_mask);
	Serial.print("config-data: ");
	Serial.println(config_data);

	// send first 5 bits
	for (config_data_bit = 0; config_data_bit <= 4; config_data_bit++) {
		if (config_data & _BV(15)) {
			spi_out_high();
		} else {
			spi_out_low();
		}
		pulse_spi_clk();
		config_data <<= 1;
	}

	spi_latch_high();

	// send bits 5..14
	for (config_data_bit = 5; config_data_bit <= 14; config_data_bit++) {
		if (config_data & _BV(15)) {
			spi_out_high();
		} else {
			spi_out_low();
		}
		pulse_spi_clk();
		config_data <<= 1;
	}

	// send last bit
	if (config_data & _BV(15)) {
		spi_out_high();
	} else {
		spi_out_low();
	}
	spi_clk_high();
	spi_latch_low();
	spi_clk_low();
}
