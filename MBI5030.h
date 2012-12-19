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

#ifndef _MBI5030_H_
#define _MBI5030_H_

// MBI5030 configuration regiter options
// First line of each #define-block: default value

// Thermal error flag (read)
// Register bit: E
// - UNTESTED -
#define THERMAL_ERROR 0x4000

// PWM resolution (read/write)
// Register bit: D
// - WORKS -
#define PWM_16BIT 0x0000
#define PWM_12BIT 0x2000

// PWM mode: normal / scramble (read/write)
// Register bit: C
// - WORKS - 
#define PWM_MODE_NORMAL	  0x0000
#define PWM_MODE_SCRAMBLE 0x1000

// DATA SYNC mode: auto / manual (read/write)
// Register bit: A
// - UNTESTED -
#define DATA_SYNC_AUTO    0x0000
#define DATA_SYNC_MANUAL  0x0400

// Thermal protection (read/write)
// Register bit: 1
// - UNTESTED -
#define THERMAL_PROTECTION_OFF 0x0000
#define THERMAL_PROTECTION_ON  0x0002

// Missing grayscale-clock shutdown
// Register bit: 0
// - UNTESTED -
#define MISSING_GSCLK_DET_ON  0x0000
#define MISSING_GSCLK_DET_OFF 0x0001

extern "C" {
#include <stdint.h>
}

class MBI5030 {
 public:
	MBI5030(uint8_t spi_out_pin, uint8_t spi_in_pin, uint8_t spi_clk_pin,
		uint8_t spi_latch_pin);
	void spi_init(void);
	void update(uint16_t * pwm_data);
	uint16_t read_error_report(void);
	uint16_t read_config(void);
	void write_config(uint16_t config_mask, uint8_t current_gain);

 private:
	volatile uint8_t *_spi_out_DIR;
	volatile uint8_t *_spi_out_PORT;
	uint8_t _spi_out_pinmask;

	volatile uint8_t *_spi_in_DIR;
	volatile uint8_t *_spi_in_PIN;
	uint8_t _spi_in_pinmask;

	volatile uint8_t *_spi_clk_DIR;
	volatile uint8_t *_spi_clk_PORT;
	uint8_t _spi_clk_pinmask;

	volatile uint8_t *_spi_latch_DIR;
	volatile uint8_t *_spi_latch_PORT;
	uint8_t _spi_latch_pinmask;

	uint8_t _spi_out_pin;
	uint8_t _spi_in_pin;
	uint8_t _spi_clk_pin;
	uint8_t _spi_latch_pin;

	inline void spi_clk_high(void) __attribute__ ((always_inline));
	inline void spi_clk_low(void) __attribute__ ((always_inline));
	inline void spi_out_high(void) __attribute__ ((always_inline));
	inline void spi_out_low(void) __attribute__ ((always_inline));
	inline void spi_latch_high(void) __attribute__ ((always_inline));
	inline void spi_latch_low(void) __attribute__ ((always_inline));
	inline void pulse_spi_clk(void) __attribute__ ((always_inline));

	void enable_error_detection(void);
	void prepare_error_report(void);
	uint16_t read_register(void);
	void prepare_config_read(void);
};

#endif
