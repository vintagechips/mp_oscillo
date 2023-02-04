#include "system.h"

uint8_t adv[SAMPLE]; // Wave form
uint8_t max;
uint8_t min;
uint8_t pos; // Draw start position
uint8_t count;

void __interrupt() ISR(){
	if(TMR2IF){
		TMR2IF = 0;
		GO_nDONE = 0;
		if(count < SAMPLE){
			adv[count++] = ADRESH;
			GO_nDONE = 1;
		}
		else
			TMR2ON = 0;
	}
}

void adc_init(){
	// Voltage input pin
	ANSA4 = 1; // Analog function
	TRISA4 = 1; // Set as input
	ADCON0 = 4 << 2; // Select AN4(RA4)

	// AD converter
	ADCON1 = 2 << 4; // Fosc/32, 1usec/bit
	ADON = 1;
	
	// Timer2
	T2CON = 0b00001001; // Prescale 1/4, postscale 1/2
//	T2CON = 0b01111011; // Prescale 1/16, postscale 1/64
	TMR2IF = 0;
	TMR2IE = 1;
	PEIE = 1;
	GIE = 1;
}

void adc_sweep(){
	PR2 = us - 1; // Sampling preod

	for(count = 0; count < SAMPLE; count++)
		adv[count] = 0;
	count = 0;
	ADIF = 0;

	GO_nDONE = 1;
	TMR2ON = 1; // 0.5usec
	while(TMR2ON);
}

void adc_analyze(){
	uint8_t trg;

	max = 0;
	min = 255;
	for(pos = 0; pos < SAMPLE; pos++){
		if(adv[pos] < 253) adv[pos] += 2; // rounding half up
		adv[pos] >>= 2; // Use upper 6 bits

		if(adv[pos] > max)
			max = adv[pos];
		if(adv[pos] < min)
			min = adv[pos];
	}

	trg = max / 2 + min / 2; // Trigger level

	// Search period
	for(pos = 0; pos < SAMPLE; pos++){
		if((adv[pos - 1] > trg) && (adv[pos] <= trg))
		break;
	}
	if(pos == SAMPLE){ // Can't find
		pod = 0; // Error code
		pos = 0; // Safe value
		return;
	}
	
	// Count period
	for(pos++, pod = 0; pos < SAMPLE; pos++){
		if((adv[pos - 1] > trg) && (adv[pos] <= trg))
		break;
		pod++;
	}

	// Search sync position
	for(pos = SCREEN / 2; pos < SAMPLE; pos++){
		if((adv[pos - 1] > trg) && (adv[pos] <= trg))
			break;
	}
	pos -= (SCREEN / 2);
}