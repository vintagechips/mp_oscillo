// Minimal parts Oscilloscope
// Author: Tetsuya Suzuki

#include "system.h"

uint8_t us;  // Sampling interval(unit us)
uint8_t pod; // Preod (unit samples)

#define TOL 16 // Sync tolerance

void main(void) {
   adc_init();  // ADC initialize
   i2c_init();  // I2C initialize
   oled_init(); // OLED initialize
   us = 48;     // Default Sampling interval 48us

   while(1){
        adc_sweep();   // Get wave form
        adc_analyze(); // Get parameters
        
        // Adjustment of sampling interval
        if(pod == 0){
            if(us < 239) us += 16;
        } else
        if(pod > SCREEN){
            if(us < 255) us++;
        } else
        if(pod < SCREEN - TOL) {
            if(us > 2) us--;
        }

        // Draw screen
        oled_label(); // Draw label area
        oled_draw();  // Draw graph area
    }
}
