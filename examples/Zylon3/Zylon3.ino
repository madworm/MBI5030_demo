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
//
// 1) MBI5030 with internal LEDs - Jumper in place!
//                                                
//                                 +----------------------+
//                                 |                      |
//                                 |      +--------+      |
//                                 |      |XXXXXXXX|      |
//                                 |      +--------+      |
//                                 |       LED-sel.       |
//                                 |                      |
//                                 |                      |
//                         GND O---O 1  GND      VCC   24 O---O 5V
//                                 |                      |
//    Arduino Uno pin #14 (A0) O---O 2  SDI      R-EXT 23 O
//                                 |                      |
//    Arduino Uno pin #15 (A1) O---O 3  DCLK     SDO   22 O---O Arduino Uno pin #17 (A3)
//                                 |                      |
//    Arduino Uno pin #16 (A2) O---O 4  LE       GCLK  21 O---O Arduino Uno pin #8 
//                                 |                      |
//                                 O 5  LED0     LED15 20 O
//                                 |                      |
//                                 O 6  LED1     LED14 19 O
//                                 |                      |
//                                 O 7  LED2     LED13 18 O
//                                 |                      |
//                                 O 8  LED3     LED12 17 O
//                                 |                      |
//                                 O 9  LED4     LED11 16 O
//                                 |                      |
//                                 O 10 LED5     LED10 15 O
//                                 |                      |
//                                 O 11 LED6     LED9  14 O
//                                 |                      |
//                                 O 12 LED7     LED8  13 O
//                                 |                      |
//                                 +----------------------+
//                   
//                        
// 2) MBI5030 with external LEDs - Jumper pulled!                     
//                                                
//                                 +----------------------+
//                                 |                      |
//                                 |      +--------+      |
//                                 |      |        |      |
//                                 |      +--------+      |
//                                 |       LED-sel.       |
//                                 |                      |
//                                 |                      |
//                         GND O---O 1  GND      VCC   24 O---O 5V
//                                 |                      |
//    Arduino Uno pin #14 (A0) O---O 2  SDI      R-EXT 23 O
//                                 |                      |
//    Arduino Uno pin #15 (A1) O---O 3  DCLK     SDO   22 O---O Arduino Uno pin #17 (A3)
//                                 |                      |
//    Arduino Uno pin #16 (A2) O---O 4  LE       GCLK  21 O---O Arduino Uno pin #8 
//                                 |                      |
//                        +--->|---O 5  LED0     LED15 20 O---|<---+
//                        |        |                      |        | 
//                        o--->|---O 6  LED1     LED14 19 O---|<---o
//                        |        |                      |        | 
//                        o--->|---O 7  LED2     LED13 18 O---|<---o    N:  number of LEDs per channel
//                        |        |                      |        |    Vf: LED forward voltage 
//                        o--->|---O 8  LED3     LED12 17 O---|<---o         
//                        |        |                      |        |    V-LED: abs. max. 17V
//                        o--->|---O 9  LED4     LED11 16 O---|<---o
//                        |        |                      |        |
//                        o--->|---O 10 LED5     LED10 15 O---|<---o
//                        |        |                      |        |
//                        o--->|---O 11 LED6     LED9  14 O---|<---o
//                        |        |                      |        |
//                        o--->|---O 12 LED7     LED8  13 O---|<---o---O V-LED ( approx. N x Vf + 0.7V )
//                        |        |                      |        |
//                        |        +----------------------+        |
//                        |                                        |
//                        +----------------------------------------+
//

#include <avr/io.h>
#include <stdint.h>
#include <MBI5030.h>

#define USE_12BIT_PWM

#ifdef USE_16BIT_PWM
#define BRIGHTNESS_MAX 65535
#define BRIGHTNESS_STEPSIZE 256
#define FADE_DELAY 1
#endif

#ifdef USE_12BIT_PWM
#define BRIGHTNESS_MAX 4095
#define BRIGHTNESS_STEPSIZE 64
#define FADE_DELAY 1
#endif

MBI5030 chip1(14, 17, 15, 16);	// spi_out, spi_in, spi_clk, spi_latch
uint16_t pwm_data[16];

#define ZYLON_DELAY 35

void setup(void)
{
	memset(pwm_data, 0x00, sizeof(pwm_data)); // fill 32 bytes with 0s
	chip1.spi_init();

	//
	// The current-set resistor 'R-EXT' of the MBI5030 should be chosen such
	// that the maximum current is only reached if the digital current gain
	// is set to '2'. That way the chip should always operate within specs.
	// 
	// The chip starts up with a gain setting of '1', which means only half
	// of the maximum current is available. Next we'll change the configuration
	// register to get a gain of '2' for full brightness.
	//
	chip1.write_config(0x0000 | PWM_12BIT, 0xFF);	// 1st number: blank configuration bits (see header file of lib), 2nd number: current gain
	chip1.update(pwm_data);
}

void loop(void)
{
	decay_brightness(24); // adapt this to ZYLON_DELAY
	zylon_step();
	chip1.update(pwm_data);
}

void decay_brightness(uint16_t steps)
{
	uint8_t counter;
	for (counter = 0; counter <= 15; counter++) {
		if (pwm_data[counter] > steps) {
			pwm_data[counter] -= steps;
		} else {
			pwm_data[counter] = 0;
		}
	}
}

void zylon_step(void)
{
	static int8_t position = 0;
	static uint8_t direction = 1;
        static uint32_t last_run = 0;

        if( (millis()-last_run) > ZYLON_DELAY ) {

	if (direction == 1) {
		pwm_data[position] = BRIGHTNESS_MAX;
		position++;
		if (position > 15) {
			direction = 0;
			position -= 2;
			return;
		}
	}

	if (direction == 0) {
		pwm_data[position] = BRIGHTNESS_MAX;
		position--;
		if (position < 0) {
			direction = 1;
			position += 2;
			return;
		}
	}

        last_run = millis();

        }
}
