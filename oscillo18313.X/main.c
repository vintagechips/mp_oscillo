// Minimal parts Oscilloscope
// Author: Tetsuya Suzuki

#include "system.h"

uint8_t us;  // Sampling interval(unit us)
uint8_t pod; // Preod (unit samples)

#define AUTO 1
#define WAVE 2
#define Hz 1000
#define TOL 32 // Sync tolerance
#define Dms 100 // Power on delay (msec)

#define Sns ((1000000UL / 96 / Hz * WAVE) & 0xff)

void main(void) {
	adc_init();  // ADC initialize
	i2c_init();  // I2C initialize
	__delay_ms(Dms);
	oled_init(); // OLED initialize

	if(Sns < 10) us =10;
	else us = Sns;

	while(1){
		adc_sweep();   // Get wave form
		adc_analyze(); // Get parameters
		oled_label(); // Draw label area

#if AUTO
		// Adjustment of sampling interval
		if(pod == 0){
			if(us < 239 && us > 26) us += 16;
		} else
		if(pod > SCREEN / WAVE){
			if(us < 255) us++;
		} else
		if(pod < SCREEN / WAVE - TOL) {
			if(us > 10) us--;
		}
#endif
		oled_draw();  // Draw graph area, pos broken
	}
}
