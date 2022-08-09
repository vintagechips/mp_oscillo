#include "system.h"

void i2c_init(){
#ifdef RA1SCL
    // RA1(SCL)
    ANSA1 = 0; // Desable analog
    TRISA1 = 1; // Set as input
    WPUA1 = 1; // Week pullup
    RA1PPS = 24; // Source is SCK/SCL
    SSP1CLKPPS = 1; // CLK1 in->RA1
#else
    // RA1(SCL)
    ANSA5 = 0; // Desable analog
    TRISA5 = 1; // Set as input
    WPUA5 = 1; // Week pullup
    RA5PPS = 24; // Source is SCK/SCL
    SSP1CLKPPS = 5; // CLK1 in->RA5
#endif

    // RA2(SDA)
    ANSA2 = 0; // Desable analog
    TRISA2 = 1; // Set as input
    WPUA2 = 1; // Week pullup
    RA2PPS = 25; // Source is SDA
    SSP1DATPPS = 2; // DAT1 in->RA2

    // I2C Mode
    SSP1STAT = 0x00; // Clear status
    SSP1CON1 = 0x08; // I2C Master mode
    SSP1CON2 = 0x00; // Default I2C mode
	SSP1ADD  = _XTAL_FREQ / (4 * _I2C_FREQ) - 1; // Set clock

    SSPEN = 1; // I2C enable
}

void i2c_start(){
    SEN = 1; // Start condition
    while(SEN); // Wait for start
}

void i2c_stop(){
    PEN = 1; // Stop condition
    while(PEN); // Wait for Stop
}

void i2c_write(uint8_t d){
    SSP1IF = 0; // Clear flag
    SSP1BUF = d; // Write
    while(!SSP1IF); // Wait for write done
}

void oled_init(){
    i2c_start();
    i2c_write(OLED_ADRS); // OLED slave address
    i2c_write(0x00); // Control byte Co=0, DC=0
    i2c_write(0x8d); // Set charge pump
    i2c_write(0x14); // Enable charge pump
    i2c_write(0x81); // Set Contrast Control
    i2c_write(0xff); // Max contrast
    i2c_write(0xc8); // Upside down
    i2c_write(0xa1); // Left side right
    i2c_write(0xaf); // Display ON
    i2c_stop();
}

// Display font
void oled_putch(uint8_t c){
	if(c < 32 | c > 94)
        c = 0;
	else
        c -= 32;

  	c <<= 2;
	i2c_write(font[c++]);
	i2c_write(font[c++]);
	i2c_write(font[c++]);
	i2c_write(font[c]);
}

// Display kigo
void oled_putk(uint8_t c){
	c <<= 2;
	i2c_write(kigo[c++]);
	i2c_write(kigo[c++]);
	i2c_write(kigo[c++]);
	i2c_write(kigo[c]);
}

// Display fonts
void oled_puts(char *s){
    while(*s)
        oled_putch(*s++);
}

// Display 3 decimal digits
void oled_putn(uint16_t n){
    uint8_t i;
    uint8_t c[3];
    
    if(n == 0){
       oled_puts("  0");
       return;
    }
    
    for(i = 0; (i < 3) && n; i++){
        c[i] = (n % 10) + 48;
        n /= 10;
    }
    
    for(; i < 3; i++)
        c[i] = ' ';
    
    while(i--)
        oled_putch(c[i]);
}

// Display decimal #.# form
void oled_putn_x10(uint16_t n){
    oled_putch((n / 10 % 10) + 48);
    oled_putch('.');
    oled_putch((n % 10) + 48);
}

// Display decimal with us or ms
void oled_label_t(uint16_t t){
    if(t < 1000){
        oled_putn(t);
        oled_putk(5); // u
    } else {
        t /= 100;
        oled_putn_x10(t);
        oled_putk(3); // m
    }
    oled_putk(4); // s
    oled_putch(' ');
}

// Display volt
void oled_label_v(uint8_t n){
    uint16_t v;
    
    oled_putch(':');
    v = n * 79 / 100;
    oled_putn_x10(v);
    oled_puts("V  ");
}

// Draw label area
void oled_label(){
    i2c_start();
    i2c_write(OLED_ADRS); // OLED slave address
    i2c_write(0x00); // Control byte Co=0, DC=0
    i2c_write(0x20); // Set memory addressing mode
    i2c_write(0x00); // Horizontal addressing mode
    i2c_write(0x21); // Set column address
    i2c_write(0);    // Column start address 0
    i2c_write(31);   // Column end address
    i2c_write(0x22); // Set page address
    i2c_write(0);    // Page start address 0
    i2c_write(7);    // Page end address 7d
    i2c_stop();

    i2c_start();
    i2c_write(OLED_ADRS); // OLED slave address
    i2c_write(0x40); // Control byte Co=0, DC=1
    oled_puts("OSCILLO ");
    oled_puts("SCOPE   ");
    oled_puts("------- ");

    oled_puts("X:");
    oled_label_t(us * 24);
    
    oled_puts("Y:1.0V  ");

    oled_putk(0); // Kigo max
    oled_label_v(max);

    oled_putk(1); // Kigo min
    oled_label_v(min);

    oled_puts("P:");
    oled_label_t((uint16_t)pod * us);
    i2c_stop();
}

// Draw graph area
void oled_draw(){
    uint8_t x;
    uint8_t y;
    uint8_t i;
    uint8_t bmp;
    uint8_t top, bot;

    i2c_start();
    i2c_write(OLED_ADRS); // OLED slave address
    i2c_write(0x00); // Control byte Co=0, DC=0
    i2c_write(0x20); // Set memory addressing mode
    i2c_write(0x01); // Virtical addressing mode
    i2c_write(0x21); // Set column address
    i2c_write(32);   // Column start address 0
    i2c_write(127);  // Column end address
    i2c_write(0x22); // Set page address
    i2c_write(0);    // Page start address
    i2c_write(7);    // Page end address
    i2c_stop();

    i2c_start();
    i2c_write(OLED_ADRS); // OLED slave address
    i2c_write(0x40); // Control byte Co=0, DC=1

    for(i = 0; i < 8; i++)
        i2c_write(0x55); // Draw virtual line

    for(x = pos + 1; x < pos + SCREEN; x++){ // Each virtual line

        // max and min
        top = adv[x];
        bot = adv[x - 1];
        if(top < bot){
            top = adv[x - 1];
            bot = adv[x];
        }

        // virtical line
        y = 64;
        while(y){ //
            bmp = 0;
            for(i = 0; i < 8; i++){
               bmp >>= 1;
               if((y <= top) && (y >= bot))
                   bmp |= 0b10000000;
               
               // Add horizontal line
                if(((y % 12) == 3) && (((pos & 1) && (x & 1)) || (((pos & 1) == 0) && ((x & 1) == 0)))){
                   bmp |= 0b10000000;
                }
               y--;
            };

            // Add virtical line
            if(((x - pos) % 24) == 0){
                bmp |= 0x55;
            }

            i2c_write(bmp); //
        }
    };

    i2c_stop();
}

const uint8_t font[] = {
	0x00,0x00,0x00,0x00, /* 000 032   */
	0x00,0x2e,0x00,0x00, /* 001 033 ! */
	0x06,0x00,0x06,0x00, /* 002 034 " */
	0x14,0x3e,0x14,0x00, /* 003 035 # */
	0x2c,0x3e,0x14,0x00, /* 004 036 $ */
	0x12,0x08,0x24,0x00, /* 005 037 % */
	0x1c,0x2a,0x34,0x00, /* 006 038 & */
	0x04,0x02,0x00,0x00, /* 007 039 ' */
	0x1c,0x22,0x00,0x00, /* 008 040 ( */
	0x00,0x22,0x1c,0x00, /* 009 041 ) */
	0x2a,0x1c,0x2a,0x00, /* 010 042 * */
	0x08,0x3e,0x08,0x00, /* 011 043 + */
	0x20,0x10,0x00,0x00, /* 012 044 , */
	0x08,0x08,0x08,0x00, /* 013 045 - */
	0x00,0x20,0x00,0x00, /* 014 046 . */
	0x20,0x1c,0x02,0x00, /* 015 047 / */
	0x1c,0x22,0x1c,0x00, /* 016 048 0 */
	0x00,0x04,0x3e,0x00, /* 017 049 1 */
	0x32,0x2a,0x24,0x00, /* 018 050 2 */
	0x2a,0x2a,0x14,0x00, /* 019 051 3 */
	0x0e,0x08,0x3e,0x00, /* 020 052 4 */
	0x2e,0x2a,0x12,0x00, /* 021 053 5 */
	0x1c,0x2a,0x12,0x00, /* 022 054 6 */
	0x22,0x12,0x0e,0x00, /* 023 055 7 */
	0x14,0x2a,0x14,0x00, /* 024 056 8 */
	0x24,0x2a,0x1c,0x00, /* 025 057 9 */
	0x00,0x14,0x00,0x00, /* 026 058 : */
	0x20,0x14,0x00,0x00, /* 027 059 ; */
	0x08,0x14,0x22,0x00, /* 028 060 < */
	0x14,0x14,0x14,0x00, /* 029 061 = */
	0x22,0x14,0x08,0x00, /* 030 062 > */
	0x02,0x2a,0x04,0x00, /* 031 063 ? */
	0x3a,0x2a,0x3e,0x00, /* 032 064 @ */
	0x3c,0x12,0x3c,0x00, /* 033 065 A */
	0x3e,0x2a,0x14,0x00, /* 034 066 B */
	0x1c,0x22,0x22,0x00, /* 035 067 C */
	0x3e,0x22,0x1c,0x00, /* 036 068 D */
	0x3e,0x2a,0x2a,0x00, /* 037 069 E */
	0x3e,0x0a,0x0a,0x00, /* 038 070 F */
	0x1c,0x22,0x3a,0x00, /* 039 071 G */
	0x3e,0x08,0x3e,0x00, /* 040 072 H */
	0x22,0x3e,0x22,0x00, /* 041 073 I */
	0x20,0x22,0x1e,0x00, /* 042 074 J */
	0x3e,0x08,0x34,0x00, /* 043 075 K */
	0x3e,0x20,0x20,0x00, /* 044 076 L */
	0x3e,0x04,0x3e,0x00, /* 045 077 M */
	0x3e,0x0c,0x18,0x3e, /* 046 078 N */
	0x3e,0x22,0x3e,0x00, /* 047 079 O */
	0x3e,0x0a,0x04,0x00, /* 048 080 P */
	0x1c,0x32,0x2c,0x00, /* 049 081 Q */
	0x3e,0x0a,0x34,0x00, /* 050 082 R */
	0x24,0x2a,0x12,0x00, /* 051 083 S */
	0x02,0x3e,0x02,0x00, /* 052 084 T */
	0x3e,0x20,0x3e,0x00, /* 053 085 U */
	0x1e,0x20,0x1e,0x00, /* 054 086 V */
	0x3e,0x10,0x3e,0x00, /* 055 087 W */
	0x36,0x08,0x36,0x00, /* 056 088 X */
	0x06,0x38,0x06,0x00, /* 057 089 Y */
	0x32,0x2a,0x26,0x00, /* 058 090 Z */
	0x00,0x3e,0x22,0x00, /* 059 091 [ */
	0x1a,0x3c,0x1a,0x00, /* 060 092 \ */
	0x22,0x3e,0x00,0x00, /* 061 093 ] */
	0x04,0x02,0x04,0x00, /* 062 094 ^ */
	0x20,0x20,0x20,0x00  /* 063 095 _ */
};

const uint8_t kigo[] = {
	0x04,0x3e,0x04,0x00, /*  */
	0x10,0x3e,0x10,0x00, /*  */
  	0x3e,0x10,0x28,0x00, /* 02 107 k */
  	0x3c,0x3c,0x38,0x00, /* 03 109 m */
  	0x28,0x34,0x14,0x00, /* 04 115 s */
	0x7c,0x20,0x3c,0x00, /* 05 u */
	0x24,0x34,0x2c,0x00, /* 06 122 z */

};
