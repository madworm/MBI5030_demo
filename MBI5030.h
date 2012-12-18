#ifndef _MBI5030_H_
#define _MBI5030_H_

extern "C" {
	#include <stdint.h>
}

class MBI5030 {

	public:
		MBI5030(uint8_t spi_out_pin, uint8_t spi_in_pin, uint8_t spi_clk_pin, uint8_t spi_latch_pin);
		void spi_init(void);
		void update(uint16_t * pwm_data);
		uint16_t read_error_report(void);
		uint16_t read_config(void);
		void write_config(uint16_t config_mask, uint8_t current_gain);
		
	private:
		volatile uint8_t * _spi_out_DIR;
		volatile uint8_t * _spi_out_PORT;
			 uint8_t   _spi_out_pinmask;

		volatile uint8_t * _spi_in_DIR;
		volatile uint8_t * _spi_in_PIN;
			 uint8_t   _spi_in_pinmask;

		volatile uint8_t * _spi_clk_DIR;
		volatile uint8_t * _spi_clk_PORT;
			 uint8_t   _spi_clk_pinmask;

		volatile uint8_t * _spi_latch_DIR;
		volatile uint8_t * _spi_latch_PORT;
			 uint8_t   _spi_latch_pinmask;

		uint8_t _spi_out_pin;
		uint8_t _spi_in_pin;
		uint8_t _spi_clk_pin;
		uint8_t _spi_latch_pin;
		
		inline void spi_clk_high(void) __attribute__((always_inline));
		inline void spi_clk_low(void) __attribute__((always_inline));
		inline void spi_out_high(void) __attribute__((always_inline));
		inline void spi_out_low(void) __attribute__((always_inline));
		inline void spi_latch_high(void) __attribute__((always_inline));
		inline void spi_latch_low(void) __attribute__((always_inline));
		inline void pulse_spi_clk(void) __attribute__((always_inline));
		
		void enable_error_detection(void);
		void prepare_error_report(void);
		uint16_t read_register(void);
		void prepare_config_read(void);
};

#endif
