//
// To get a suitably high GSCLK frequency, the CLKO-FUSE of the ATMega168/328 has been programmed
// It will output it's system clock on PB0 ("digital pin" #8)
//
// Diecimila + ATmega168: lfuse: 0xBF
//                        hfuse: 0xDD
//                        efuse: 0x00 (or 0xF8)
//
// Uno + ATmega328:       lfuse: 0xBF
//                        hfuse: 0xDE
//                        efuse: 0x05 (or 0xFD)
//                        
// To revert to Arduino's default FUSE settings, change 0xBF to 0xFF.
//
// Use: "http://www.engbedded.com/fusecalc" and ".../arduino-XXX/hardware/arduino/boards.txt"
//
// Make sure to get the latest MBI5030 datasheet (at least version Jan. 2009)!
// 

#include <avr/io.h>
#include <stdint.h>
#include <Arduino.h>
#include "MBI5030.h"

MBI5030::MBI5030(uint8_t spi_out_pin, uint8_t spi_in_pin, uint8_t spi_clk_pin,
		 uint8_t spi_latch_pin)
{
	_spi_out_pin = spi_out_pin;
	_spi_in_pin = spi_in_pin;
	_spi_clk_pin = spi_clk_pin;
	_spi_latch_pin = spi_latch_pin;

	_spi_out_DIR = portModeRegister(digitalPinToPort(_spi_out_pin));
	_spi_out_PORT = portOutputRegister(digitalPinToPort(_spi_out_pin));
	_spi_out_pinmask = digitalPinToBitMask(_spi_out_pin);

	_spi_in_DIR = portModeRegister(digitalPinToPort(_spi_in_pin));
	_spi_in_PIN = portInputRegister(digitalPinToPort(_spi_in_pin));
	_spi_in_pinmask = digitalPinToBitMask(_spi_in_pin);

	_spi_clk_DIR = portModeRegister(digitalPinToPort(_spi_clk_pin));
	_spi_clk_PORT = portOutputRegister(digitalPinToPort(_spi_clk_pin));
	_spi_clk_pinmask = digitalPinToBitMask(_spi_clk_pin);

	_spi_latch_DIR = portModeRegister(digitalPinToPort(_spi_latch_pin));
	_spi_latch_PORT = portOutputRegister(digitalPinToPort(_spi_latch_pin));
	_spi_latch_pinmask = digitalPinToBitMask(_spi_latch_pin);
}

void MBI5030::spi_init(void)
{
	*_spi_out_DIR |= _spi_out_pinmask;
	*_spi_out_PORT &= ~_spi_out_pinmask;

	*_spi_in_DIR &= ~_spi_in_pinmask;

	*_spi_clk_DIR |= _spi_clk_pinmask;
	*_spi_clk_PORT &= ~_spi_clk_pinmask;

	*_spi_latch_DIR |= _spi_latch_pinmask;
	*_spi_latch_PORT &= ~_spi_latch_pinmask;
}

inline void MBI5030::spi_out_high(void)
{
	*_spi_out_PORT |= _spi_out_pinmask;
}

inline void MBI5030::spi_out_low(void)
{
	*_spi_out_PORT &= ~_spi_out_pinmask;
}

inline void MBI5030::spi_clk_high(void)
{
	*_spi_clk_PORT |= _spi_clk_pinmask;
}

inline void MBI5030::spi_clk_low(void)
{
	*_spi_clk_PORT &= ~_spi_clk_pinmask;
}

inline void MBI5030::spi_latch_high(void)
{
	*_spi_latch_PORT |= _spi_latch_pinmask;
}

inline void MBI5030::spi_latch_low(void)
{
	*_spi_latch_PORT &= ~_spi_latch_pinmask;
}

inline void MBI5030::pulse_spi_clk(void)
{
	*_spi_clk_PORT |= _spi_clk_pinmask;
	*_spi_clk_PORT &= ~_spi_clk_pinmask;
}

void MBI5030::update(uint16_t * pwm_data)
{
	uint8_t data_word;
	uint8_t data_word_bit;
	uint16_t pwm_data_tmp;

	//
	// measure how long one update-cycle takes
	//
	// uint32_t start;
	// uint32_t stop;
	// start = micros();

	// send the first 15 words with "data-latch"
	// the input shift register is 16bit wide
	// it requires a "data-latch" to move the data to
	// the chip-internal buffers

	for (data_word = 0; data_word <= (16 - 2); data_word++) {

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

	//
	// measure how long one update-cycle takes
	//      
	//stop = micros();
	//Serial.println(stop-start);
}

void MBI5030::enable_error_detection(void)
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

void MBI5030::prepare_error_report(void)
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

uint16_t MBI5030::read_error_report(void)
{
	enable_error_detection();
	prepare_error_report();
	return read_register();
}

uint16_t MBI5030::read_register(void)
{
	uint16_t register_status = 0;
	uint8_t register_status_bit;

	// read bits 0-14
	for (register_status_bit = 0; register_status_bit <= 14;
	     register_status_bit++) {
		if (*_spi_in_PIN & _spi_in_pinmask) {
			register_status |= _BV(0);
		} else {
			// already full with zeros
		}
		pulse_spi_clk();
		register_status <<= 1;
	}

	// read bit 15
	if (*_spi_in_PIN & _spi_in_pinmask) {
		register_status |= _BV(0);
	} else {
		// already full with zeros
	}

	return register_status;
}

void MBI5030::prepare_config_read(void)
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

uint16_t MBI5030::read_config(void)
{
	prepare_config_read();
	return read_register();
}

void MBI5030::write_config(uint16_t config_mask, uint8_t current_gain)
{

	uint16_t current_gain_mask = (((uint16_t) (current_gain)) << 2);
	uint16_t config_data = (0x0000 | config_mask | current_gain_mask);
	uint8_t config_data_bit;

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
