#ifndef _MBI5030H_
#define _MBI5030H_

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__)
#define SPI_OUT_DIR DDRC
#define SPI_OUT_PORT PORTC
#define SPI_OUT_pin PC0

#define SPI_CLK_DIR DDRC
#define SPI_CLK_PORT PORTC
#define SPI_CLK_pin PC1

#define SPI_LATCH_DIR DDRC
#define SPI_LATCH_PORT PORTC
#define SPI_LATCH_pin PC2

#define SPI_IN_DIR DDRC
#define SPI_IN_PORT PORTC
#define SPI_IN_PIN PINC
#define SPI_IN_pin PC3
#else
#error DOH! Needs ATmega168 or ATmega328
#error SPI DATA OUT: analog pin 0
#error SPI CLK     : analog pin 1
#error SPI LATCH   : analog pin 2
#error SPI DATA IN : analog pin 3
#error Go and fix it yourself my friend!
#endif

#define PWM_BUFFER_SIZE 16

#ifdef USE_PWM_12BIT
#define BRIGHTNESS_STEPSIZE 16
#define BRIGHTNESS_MAX 4095	// 12bit PWM
#define FADE_DELAY 2
#endif

#ifdef USE_PWM_16BIT
#define BRIGHTNESS_STEPSIZE 128
#define BRIGHTNESS_MAX 65535
#define FADE_DELAY 1
#endif

// definitions of configuration bits of the MBI5030 config register
// 1st lines: DEFAULT values
#define PWM_16BIT                  0x0000
#define PWM_12BIT                  0x2000

// doesn't seem to work with my chip ;-(
#define MISSING_GSCLK_SHUTDOWN_ON  0x0000
#define MISSING_GSCLK_SHUTDOWN_OFF 0x0001

// not tested yet
#define THERMAL_I_LIMIT_OFF        0x0000
#define THERMAL_I_LIMIT_ON         0x0002

extern uint16_t pwm_buffer[PWM_BUFFER_SIZE];

void spi_setup(void);
void update_pwm(uint16_t * pwm_data);
void enable_error_detection(void);
void prepare_error_report(void);
uint16_t read_register(void);
void prepare_config_read(void);
void write_config(uint16_t config_mask, uint8_t current_gain);

#endif
