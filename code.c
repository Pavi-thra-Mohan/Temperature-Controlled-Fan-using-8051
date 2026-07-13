/*******************************************************************************
 * Project: Temperature Controlled Fan & Heater using 8051
 * Compiler: Keil C51
 * Description: 
 *   Reads temperature using an ADC (ADC0804) connected to P3.
 *   Displays the temperature, heater status, and fan speed level on a 16x2 LCD.
 *   Controls a motor (Fan) and two Heater stages based on threshold zones.
 ******************************************************************************/
#include <reg51.h> 
/*------------------------------------------------------------------------------
 * Temperature Constants (Thresholds)
 *----------------------------------------------------------------------------*/
#define FAN_LEVEL3_MIN      35 
#define FAN_LEVEL2_MAX      35 
#define FAN_LEVEL2_MIN      30 
#define FAN_LEVEL1_MAX      30
#define FAN_LEVEL1_MIN      25
#define NORMAL_MAX          25
#define NORMAL_MIN          20
#define HEATER_LEVEL1_MAX   20
#define HEATER_LEVEL1_MIN   10
#define HEATER_LEVEL2_MAX   10
#define HEATER_LEVEL2_MIN   0
/*------------------------------------------------------------------------------
 * LCD Command Constants
 *----------------------------------------------------------------------------*/
#define LCD_MODE_8BIT       0x38  // 8-bit mode, 2 lines, 5x7 font
#define LCD_DISPLAY_ON      0x0C  // Display ON, Cursor OFF
#define LCD_CLEAR           0x01  // Clear Display
#define LCD_LINE1           0x80  // Force cursor to beginning of 1st line
#define LCD_LINE2           0xC0  // Move cursor to beginning of 2nd line
#define LCD_STATUS_POS      0x8B  // Position for status text on line 1 (11th col)
#define LCD_TEMP_POS        0x86  // Position for temperature value on line 1
/*------------------------------------------------------------------------------
 * Hardware Port Definitions
 *----------------------------------------------------------------------------*/
#define ADC_DATA P3   // Input data port from ADC0804
#define LCD_DATA P1   // Output data port to LCD
/*------------------------------------------------------------------------------
 * Pin Definitions
 *----------------------------------------------------------------------------*/
sbit rd   = P2^5;     // Read control pin for ADC
sbit wr   = P2^4;     // Write control pin for ADC
sbit INTR = P2^3;     // Interrupt pin from ADC (active low, conversion complete)
sbit rs   = P2^0;     // Register Select pin for LCD
sbit rw   = P2^1;     // Read/Write pin for LCD
sbit en   = P2^2;     // Enable pin for LCD
sbit busy = P1^7;     // Busy flag pin (LCD D7)
sbit MTR  = P2^6;     // Fan Motor control pin
sbit HTR1 = P0^0;     // Heater 1 control pin
sbit HTR2 = P0^1;     // Heater 2 control pin
/*------------------------------------------------------------------------------
 * Function Declarations
 *----------------------------------------------------------------------------*/
void msDelay(unsigned int value);
void lcdReady(void);
void lcdCmd(unsigned char value);
void lcdInit(void);
void lcdData(char value);
void lcdPrintString(char *str, unsigned int delay);
void display(char d1, char d2);
void convert(char value);
void update(char value);
char adcRead(void);
void motorcontrol(void);
/*------------------------------------------------------------------------------
 * Delay Utility Function
 *----------------------------------------------------------------------------*/
void msDelay(unsigned int value) { 
    unsigned int x, y;
    for(x = 0; x < value; x++) {
        for(y = 0; y < 1275; y++); 
    }
}
/*------------------------------------------------------------------------------
 * LCD Control Functions
 *----------------------------------------------------------------------------*/
void lcdReady(void) { 
    busy = 1;
    rs = 0;
    rw = 1;
    while(busy == 1) {
        en = 0;
        en = 1;
    }
}
void lcdCmd(unsigned char value) { 
    lcdReady();
    LCD_DATA = value;
    rs = 0;
    rw = 0;
    en = 1;
    en = 0;
}
void lcdInit(void) { 
    lcdCmd(LCD_MODE_8BIT);
    lcdCmd(LCD_DISPLAY_ON);
    lcdCmd(LCD_CLEAR);
    lcdCmd(LCD_LINE1);
}
void lcdData(char value) { 
    LCD_DATA = value;
    rs = 1;
    rw = 0;
    en = 1;
    en = 0;
}
/**
 * Helper to display a null-terminated string with a delay between characters
 */
void lcdPrintString(char *str, unsigned int delay) {
    while (*str) {
        msDelay(delay);
        lcdData(*str++);
    }
}
/*------------------------------------------------------------------------------
 * Temperature & Display Formatting
 *----------------------------------------------------------------------------*/
void display(char d1, char d2) {
    lcdData(d2);
    msDelay(30);
    msDelay(30);
    lcdData(d1);
    msDelay(30);
    lcdData(' ');
    msDelay(30);
    lcdData('C');
}
void convert(char value) {
    char y, d1, d2;
    y = value / 10;
    d1 = value % 10;
    d2 = y % 10;
    
    d1 = d1 | 0x30; // Convert to ASCII
    d2 = d2 | 0x30;
    display(d1, d2);
}
void update(char value) {
    char y, d1, d2; 
    y = value / 10; 
    d1 = value % 10;
    d2 = y % 10;
    
    d1 = d1 | 0x30;
    d2 = d2 | 0x30;
    
    lcdData(d2);
    msDelay(30);
    msDelay(30);
    lcdData(d1);
    msDelay(30);
    lcdCmd(LCD_LINE2);
}
/*------------------------------------------------------------------------------
 * Analog to Digital Converter (ADC) Read
 *----------------------------------------------------------------------------*/
char adcRead(void) {
    char value;
    wr = 0; // Trigger ADC start conversion
    wr = 1;
    while(INTR == 1); // Wait for conversion to complete
    rd = 0; // Enable read
    value = ADC_DATA; // Read data
    rd = 1; // Disable read
    return value;
}
/*------------------------------------------------------------------------------
 * Core Motor & Heater Control Logic
 *----------------------------------------------------------------------------*/
void motorcontrol(void) {
    unsigned char value;
    
    // LCD Messages
    char temperatureLabel[] = "TEMPR:";
    char fanOnText[]         = "FANON";
    char heaterOnText[]      = "HTRON";
    char speedLabel[]       = "SPEED:LEVEL "; 
    char heatLabel[]        = "HEAT:LEVEL ";  
    char normalText[]       = "NORM.";
    char idleText[]         = "FAN,HTR OFF";
    
    while(1) { 
        value = adcRead(); 
        
        /*----------------------------------------------------------------------
         * HEATING MODE (Temp < 20°C)
         *--------------------------------------------------------------------*/
        if(value < 20) { 
            lcdCmd(LCD_CLEAR);
            lcdPrintString(temperatureLabel, 50);
            convert(value); 
            
            lcdCmd(LCD_STATUS_POS);
            lcdPrintString(heaterOnText, 30);
            
            // Level 1 Heat (10°C < Temp < 20°C)
            while(value > HEATER_LEVEL1_MIN && value < HEATER_LEVEL1_MAX) {   
                MTR = 1;
                HTR2 = 1;
                HTR1 = 0;
                msDelay(1);
                
                heatLabel[10] = '1'; 
                lcdCmd(LCD_LINE2);
                lcdPrintString(heatLabel, 30);
                
                bkl5:
                value = adcRead();
                if(value > HEATER_LEVEL1_MIN && value < HEATER_LEVEL1_MAX) {
                    lcdCmd(LCD_TEMP_POS);
                    update(value);
                    goto bkl5;
                } else {
                    break;
                }
            }
            
            // Level 2 Heat (0°C <= Temp <= 10°C)
            while(value >= HEATER_LEVEL2_MIN && value <= HEATER_LEVEL2_MAX) { 
                MTR = 1;
                HTR1 = 0;
                msDelay(30);
                HTR2 = 0;
                msDelay(1);
                
                heatLabel[10] = '2';
                lcdCmd(LCD_LINE2);
                lcdPrintString(heatLabel, 30);
                
                bkl6:
                value = adcRead();
                if(value >= HEATER_LEVEL2_MIN && value <= HEATER_LEVEL2_MAX) {
                    lcdCmd(LCD_TEMP_POS);
                    update(value);
                    goto bkl6;
                } else {
                    break;
                }
            }
        } 
        
        /*----------------------------------------------------------------------
         * NORMAL STATE (20°C <= Temp < 25°C)
         *--------------------------------------------------------------------*/
        while(value >= NORMAL_MIN && value < NORMAL_MAX) { 
            HTR1 = 1;
            HTR2 = 1;
            MTR = 1;
            lcdCmd(LCD_CLEAR);
            lcdPrintString(temperatureLabel, 50);
            convert(value);
            
            lcdPrintString(normalText, 30);
            lcdCmd(LCD_LINE2);
            lcdPrintString(idleText, 30);
            
            bklbkl:
            value = adcRead();
            if(value >= NORMAL_MIN && value < NORMAL_MAX) {
                lcdCmd(LCD_TEMP_POS);
                update(value);
                goto bklbkl;
            } else {
                break;
            }
        }
        /*----------------------------------------------------------------------
         * COOLING MODE (Temp >= 25°C)
         *--------------------------------------------------------------------*/
        if(value >= 25) {
            HTR1 = 1;
            HTR2 = 1;
            lcdCmd(LCD_CLEAR);
            lcdPrintString(temperatureLabel, 50);
            convert(value);
            
            lcdCmd(LCD_STATUS_POS);
            lcdPrintString(fanOnText, 30);
            // Level 1 Fan Speed (25°C <= Temp < 30°C)
            while(value >= FAN_LEVEL1_MIN && value < FAN_LEVEL1_MAX) { 
                lcdCmd(LCD_LINE2);
                speedLabel[11] = '1';
                lcdPrintString(speedLabel, 30);
                
                bkl1:
                MTR = 0;
                msDelay(50);
                MTR = 1;
                msDelay(50);
                value = adcRead();
                if(value >= FAN_LEVEL1_MIN && value < FAN_LEVEL1_MAX) {
                    lcdCmd(LCD_TEMP_POS);
                    update(value);
                    goto bkl1;
                } else {
                    break;
                }
            }
            // Level 2 Fan Speed (30°C <= Temp < 35°C)
            while(value >= FAN_LEVEL2_MIN && value < FAN_LEVEL2_MAX) { 
                speedLabel[11] = '2';
                lcdCmd(LCD_LINE2);
                lcdPrintString(speedLabel, 30);
                
                bkl2:
                MTR = 0;
                msDelay(75);
                MTR = 1;
                msDelay(25);
                value = adcRead();
                if(value >= FAN_LEVEL2_MIN && value < FAN_LEVEL2_MAX) {
                    lcdCmd(LCD_TEMP_POS);
                    update(value);
                    goto bkl2;
                } else {
                    break;
                }
            }
            // Level 3 Fan Speed (Temp >= 35°C)
            while(value >= FAN_LEVEL3_MIN) { 
                speedLabel[11] = '3';
                lcdCmd(LCD_LINE2);
                lcdPrintString(speedLabel, 30);
                
                bkl3:
                MTR = 0;
                value = adcRead();
                if(value >= FAN_LEVEL3_MIN) {
                    lcdCmd(LCD_TEMP_POS);
                    update(value);
                    goto bkl3;
                } else {
                    break;
                }
            } 
        } 
    }
}
/*------------------------------------------------------------------------------
 * Main Function
 *----------------------------------------------------------------------------*/
void main(void) { 
    P0 = 0xff;      // Configure ports as inputs/outputs
    ADC_DATA = 0xff;// Set data port high
    INTR = 1;       // Configure interrupt pin as input
    rd = 1;
    wr = 1;
    MTR = 1;        // Keep Motor OFF initially
    HTR1 = 0;       // Toggle heaters initialization sequence
    HTR2 = 0;
    msDelay(50);
    HTR1 = 1;       // Keep Heaters OFF initially
    HTR2 = 1;
    
    lcdInit();      // Initialize LCD module
    motorcontrol(); // Start main application loop
}

