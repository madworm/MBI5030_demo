#include <avr/io.h>
#include <stdint.h>
#include <MBI5030.h>

#define USE_16BIT_PWM

#ifdef USE_16BIT_PWM
#define BRIGHTNESS_MAX 65535
#define BRIGHTNESS_STEPSIZE 128
#define FADE_DELAY 1
#endif

#ifdef USE_12BIT_PWM
#define BRIGHTNESS_MAX 4095
#define BRIGHTNESS_STEPSIZE 32
#define FADE_DELAY 1
#endif

MBI5030 chip1(14,17,15,16); // spi_out, spi_in, spi_clk, spi_latch
MBI5030 chip2(7,6,5,4);
uint16_t pwm_data[16];

void setup(void) {
  Serial.begin(9600);
  memset(pwm_data,0x0000,sizeof(pwm_data));
  chip1.spi_init();
  chip2.spi_init();
  chip1.write_config(0x0000,0x00);
  chip2.write_config(0x0000,0xFF);
  chip1.update(pwm_data);
  chip2.update(pwm_data);
}

void loop(void) {
  Serial.print("CHIP1 - LED error-status: ");
  Serial.println(chip1.read_error_report(),BIN);
  Serial.print("CHIP 1 - config register: ");
  Serial.println(chip1.read_config(),BIN);  
  Serial.print("CHIP2 - LED error-status: ");
  Serial.println(chip2.read_error_report(),BIN);
  Serial.print("CHIP2 - config register: ");
  Serial.println(chip2.read_config(),BIN);    
  uint8_t counter;
  uint16_t brightness;
   
   brightness = 0;
   while( brightness <=  BRIGHTNESS_MAX ) {
     
     for(counter = 0; counter <= 15; counter++) {
       pwm_data[counter] = brightness;
     }  
     
     chip1.update(pwm_data);
     chip2.update(pwm_data);
     
     if( brightness <= BRIGHTNESS_MAX - BRIGHTNESS_STEPSIZE ) {
       brightness += BRIGHTNESS_STEPSIZE;
     } else if( brightness == BRIGHTNESS_MAX ) {
       break;
     } else {
       brightness = BRIGHTNESS_MAX;
     }
     delay(FADE_DELAY);
   }
   
   while( brightness >= 0 ) {
     
     for(counter = 0; counter <= 15; counter++) {
       pwm_data[counter] = brightness;
     }  
     
     chip1.update(pwm_data);
     chip2.update(pwm_data);
     
     if( brightness >= 0 + BRIGHTNESS_STEPSIZE ) {     
       brightness -= BRIGHTNESS_STEPSIZE;
     } else if( brightness == 0 ) {
       break;
     } else {
       brightness = 0;
     }
     delay(FADE_DELAY);
   }
   delay(250);
}
