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

#define USE_PWM_12BIT		// make sure to set 12BIT or 16BIT mode with write_config() as well!

#include <avr/io.h>
#include <stdint.h>
#include <MBI5030.h>

void setup(void)
{
	spi_setup();		// see the header file for which pins are used
	memset(pwm_buffer, 0x0000, sizeof(pwm_buffer));	// clear the pwm_buffer
	update_pwm(pwm_buffer);	// push data to chip
	write_config(PWM_12BIT, 0xFF);	// uint16_t: various config bits - uint8_t: current gain
}

void loop(void)
{
	enable_error_detection();
	prepare_error_report();
	Serial.print("error-status: ");
	Serial.println(read_register(), BIN);
	prepare_config_read();
	Serial.print("config register: ");
	Serial.println(read_register(), DEC);

	uint8_t counter;
	static uint16_t brightness = 0;

	while (brightness <= BRIGHTNESS_MAX) {

		for (counter = 0; counter <= 15; counter++) {
			pwm_buffer[counter] = brightness;
		}

		update_pwm(pwm_buffer);

		if (brightness <= BRIGHTNESS_MAX - BRIGHTNESS_STEPSIZE) {
			brightness += BRIGHTNESS_STEPSIZE;
		} else if (brightness == BRIGHTNESS_MAX) {
			break;
		} else {
			brightness = BRIGHTNESS_MAX;
		}
		delay(FADE_DELAY);
	}

	while (brightness >= 0) {

		for (counter = 0; counter <= 15; counter++) {
			pwm_buffer[counter] = brightness;
		}

		update_pwm(pwm_buffer);

		if (brightness >= 0 + BRIGHTNESS_STEPSIZE) {
			brightness -= BRIGHTNESS_STEPSIZE;
		} else if (brightness == 0) {
			break;
		} else {
			brightness = 0;
		}
		delay(FADE_DELAY);
	}
	delay(250);
}
