/*
 * "Copyright (c) 2008 Robert B. Reese, Bryan A. Jones, J. W. Bruce ("AUTHORS")"
 * All rights reserved.
 * (R. Reese, reese_AT_ece.msstate.edu, Mississippi State University)
 * (B. A. Jones, bjones_AT_ece.msstate.edu, Mississippi State University)
 * (J. W. Bruce, jwbruce_AT_ece.msstate.edu, Mississippi State University)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice, the following
 * two paragraphs and the authors appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE "AUTHORS" BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE "AUTHORS"
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE "AUTHORS" SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE "AUTHORS" HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Please maintain this header in its entirety when copying/modifying
 * these files.
 *
 *
 */
#include "pic24_all.h"
#include <stdio.h>

/** \file
 *  Demonstrates reading the internal ADC in 12-bit mode and
 *  then sending the upper 8-bits to an external
 *  8-bit SPI DAC (MAXIM 548A)
 */

#define CONFIG_SLAVE_ENABLE() CONFIG_RB3_AS_DIG_OUTPUT()
#define SLAVE_ENABLE()        _LATB3 = 0  //low true assertion
#define SLAVE_DISABLE()       _LATB3 = 1


void configSPI1(void) {
  //spi clock = 40MHz/1*4 = 40MHz/4 = 10MHz
  SPI1CON1 = SEC_PRESCAL_1_1 |     //1:1 secondary prescale
             PRI_PRESCAL_4_1 |     //4:1 primary prescale
             CLK_POL_ACTIVE_HIGH | //clock active high (CKP = 0)
             SPI_CKE_ON          | //out changes active to inactive (CKE=1)
             SPI_MODE8_ON        | //8-bit mode
             MASTER_ENABLE_ON;     //master mode
#if (defined(__dsPIC33E__) || defined(__PIC24E__))
  //nothing to do here. On this family, the SPI1 port uses dedicated
  //pins for higher speed. The SPI2 port can be used with remappable pins.
  //you may need to add code to disable analog functionality if the SPI ports
  //are on analog-capable pins.
#else
  CONFIG_SDO1_TO_RP(11);      //use RP11 for SDO
  CONFIG_SCK1OUT_TO_RP(12);   //use RP12 for SCLK
#endif

  SPI1STATbits.SPIEN = 1;  //enable SPI mode
}
void configDAC() {
  CONFIG_SLAVE_ENABLE();       //chip select for DAC
  SLAVE_DISABLE();             //disable the chip select
}

void writeDAC (uint8_t dacval) {
  SLAVE_ENABLE();                 //assert Chipselect line to DAC
  ioMasterSPI1(0b00001010);      //control byte that enables DAC B
  ioMasterSPI1(dacval);          //write DAC value
  SLAVE_DISABLE();
}


// LED configuration
#define CONFIG_LED1() CONFIG_RA0_AS_DIG_OUTPUT()
#define LED1  _LATA0     //led1 state

// LCD configuration

#define RS_HIGH()        _LATB8 = 1
#define RS_LOW()         _LATB8 = 0
#define CONFIG_RS()      CONFIG_RB8_AS_DIG_OUTPUT()

#define RW_HIGH()        _LATB9 = 1
#define RW_LOW()         _LATB9 = 0
#define CONFIG_RW()      CONFIG_RB9_AS_DIG_OUTPUT()

#define E_HIGH()         _LATB13 = 1
#define E_LOW()          _LATB13 = 0
#define CONFIG_E()       CONFIG_RB13_AS_DIG_OUTPUT()

#define LCD4O          _LATB4
#define LCD5O          _LATB5
#define LCD6O          _LATB6
#define LCD7O          _LATB7
#define LCD7I          _RB7

#define CONFIG_LCD4_AS_INPUT() CONFIG_RB4_AS_DIG_INPUT()
#define CONFIG_LCD5_AS_INPUT() CONFIG_RB5_AS_DIG_INPUT()
#define CONFIG_LCD6_AS_INPUT() CONFIG_RB6_AS_DIG_INPUT()
#define CONFIG_LCD7_AS_INPUT() CONFIG_RB7_AS_DIG_INPUT()

#define CONFIG_LCD4_AS_OUTPUT() CONFIG_RB4_AS_DIG_OUTPUT()
#define CONFIG_LCD5_AS_OUTPUT() CONFIG_RB5_AS_DIG_OUTPUT()
#define CONFIG_LCD6_AS_OUTPUT() CONFIG_RB6_AS_DIG_OUTPUT()
#define CONFIG_LCD7_AS_OUTPUT() CONFIG_RB7_AS_DIG_OUTPUT()

#define GET_BUSY_FLAG()  LCD7I

//Configure 4-bit data bus for output
void configBusAsOutLCD(void) {
  RW_LOW();                  //RW=0 to stop LCD from driving pins
  CONFIG_LCD4_AS_OUTPUT();   //D4
  CONFIG_LCD5_AS_OUTPUT();   //D5
  CONFIG_LCD6_AS_OUTPUT();   //D6
  CONFIG_LCD7_AS_OUTPUT();   //D7
}

//Configure 4-bit data bus for input
void configBusAsInLCD(void) {
  CONFIG_LCD4_AS_INPUT();   //D4
  CONFIG_LCD5_AS_INPUT();   //D5
  CONFIG_LCD6_AS_INPUT();   //D6
  CONFIG_LCD7_AS_INPUT();   //D7
  RW_HIGH();                   // R/W = 1, for read
}

//Output lower 4-bits of u8_c to LCD data lines
void outputToBusLCD(uint8_t u8_c) {
  LCD4O = u8_c & 0x01;          //D4
  LCD5O = (u8_c >> 1)& 0x01;    //D5
  LCD6O = (u8_c >> 2)& 0x01;    //D6
  LCD7O = (u8_c >> 3)& 0x01;    //D7
}

//Configure the control lines for the LCD
void configControlLCD(void) {
  CONFIG_RS();     //RS
  CONFIG_RW();     //RW
  CONFIG_E();      //E
  RW_LOW();
  E_LOW();
  RS_LOW();
}

//Pulse the E clock, 1 us delay around edges for
//setup/hold times
void pulseE(void) {
  DELAY_US(1);
  E_HIGH();
  DELAY_US(1);
  E_LOW();
  DELAY_US(1);
}

/* Write a byte (u8_Cmd) to the LCD.
u8_DataFlag is '1' if data byte, '0' if command byte
u8_CheckBusy is '1' if must poll busy bit before write, else simply delay before write
u8_Send8Bits is '1' if must send all 8 bits, else send only upper 4-bits
*/
void writeLCD(uint8_t u8_Cmd, uint8_t u8_DataFlag,
              uint8_t u8_CheckBusy, uint8_t u8_Send8Bits) {

  uint8_t u8_BusyFlag;
  uint8_t u8_wdtState;
  if (u8_CheckBusy) {
    RS_LOW();            //RS = 0 to check busy
    // check busy
    configBusAsInLCD();  //set data pins all inputs
    u8_wdtState = _SWDTEN;  //save WDT enable state
    CLRWDT();  			   //clear the WDT timer
    _SWDTEN = 1;            //enable WDT to escape infinite wait
    do {
      E_HIGH();
      DELAY_US(1);  // read upper 4 bits
      u8_BusyFlag = GET_BUSY_FLAG();
      E_LOW();
      DELAY_US(1);
      pulseE();              //pulse again for lower 4-bits
    } while (u8_BusyFlag);
    _SWDTEN = u8_wdtState;   //restore WDT enable state
  } else {
    DELAY_MS(10); // don't use busy, just delay
  }
  configBusAsOutLCD();
  if (u8_DataFlag) RS_HIGH();   // RS=1, data byte
  else    RS_LOW();             // RS=0, command byte
  outputToBusLCD(u8_Cmd >> 4);  // send upper 4 bits
  pulseE();
  if (u8_Send8Bits) {
    outputToBusLCD(u8_Cmd);     // send lower 4 bits
    pulseE();
  }
}

// Initialize the LCD, modify to suit your application and LCD
void initLCD() {
  DELAY_MS(50);          //wait for device to settle
  writeLCD(0x20,0,0,0); // 4 bit interface
  writeLCD(0x28,0,0,1); // 2 line display, 5x7 font
  writeLCD(0x28,0,0,1); // repeat
  writeLCD(0x06,0,0,1); // enable display
  writeLCD(0x0C,0,0,1); // turn display on; cursor, blink is off
  writeLCD(0x01,0,0,1); // clear display, move cursor to home
  DELAY_MS(3);
}

//Output a string to the LCD
void outStringLCD(char *psz_s) {
  while (*psz_s) {
    writeLCD(*psz_s, 1, 1,1);
    psz_s++;
  }
}


#define VREF 3.3  //assume Vref = 3.3 volts


volatile uint16_t u16_adcVal; //Store value read from ADC
volatile uint8_t u8_dacVal; //Store value output to DAC
volatile float f_adcVal; //Store ADC value as float
volatile uint8_t Vscaled; //Scaled value to display on LCD
volatile char str[3]; //String to store value output to LCD

//ISR for Timer 2 (LED control every 100 ms)
void _ISRFAST _T2Interrupt (void) {
	//Every 100 ms
	
	if (f_adcVal < 1.7) LED1 = 1; //Turn on LED1 when Vin < 1.7 V
	else LED1 = !LED1; //Toggle LED1
	
	_T2IF = 0;
}

//ISR for Timer 3 (ADC reading every 10 ms)
void _ISRFAST _T3Interrupt (void) {
	//Every 10 ms
	
    u16_adcVal = convertADC1();   //get ADC value
    u8_dacVal = (u16_adcVal>>4) & 0x00FF;  //upper 8 bits to DAC value
    writeDAC(u8_dacVal);
    f_adcVal = u16_adcVal;
    f_adcVal = f_adcVal/4096.0 * VREF;  //convert to float 0.0 to VREF
	Vscaled = 255.0 * f_adcVal/VREF;
	
	//Output Vscaled to LCD
	writeLCD(0x01,0,0,1);  // clear LCD
	sprintf(str, "%.3d", Vscaled); //format to string
    outStringLCD(str); //output count
	
	//Clear timer interrupt bit
	_T3IF = 0;
}

//Interrupt timer configuration

#define ISR2_PERIOD     100                // in ms
void  configTimer2(void) {
  //ensure that Timer2,3 configured as separate timers.
  T2CONbits.T32 = 0;     // 32-bit mode off
  //T3CON set like this for documentation purposes.
  //could be replaced by T3CON = 0x0020
  T2CON = T2_OFF |T2_IDLE_CON | T2_GATE_OFF
          | T2_SOURCE_INT
          | T2_PS_1_64 ;  //results in T3CON= 0x0020
  PR2 = msToU16Ticks (ISR2_PERIOD, getTimerPrescale(T2CONbits)) - 1;	
  TMR2  = 0;                       //clear timer3 value
  _T2IF = 0;                       //clear interrupt flag
  _T2IP = 2;                       //choose a priority
  _T2IE = 1;                       //enable the interrupt
  T2CONbits.TON = 1;               //turn on the timer
}

#define ISR3_PERIOD     10                // in ms
void  configTimer3(void) {
  //ensure that Timer2,3 configured as separate timers.
  T2CONbits.T32 = 0;     // 32-bit mode off
  //T3CON set like this for documentation purposes.
  //could be replaced by T3CON = 0x0020
  T3CON = T3_OFF |T3_IDLE_CON | T3_GATE_OFF
          | T3_SOURCE_INT
          | T3_PS_1_64 ;  //results in T3CON= 0x0020
  PR3 = msToU16Ticks (ISR3_PERIOD, getTimerPrescale(T3CONbits)) - 1;	
  TMR3  = 0;                       //clear timer3 value
  _T3IF = 0;                       //clear interrupt flag
  _T3IP = 1;                       //choose a priority
  _T3IE = 1;                       //enable the interrupt
  T3CONbits.TON = 1;               //turn on the timer
}


int main (void) {
	configClock();                //clock configuration
	
	// Configure analog input
	CONFIG_AN1_AS_ANALOG();

	// Configure A/D to sample AN1 for 31 Tad periods in 12-bit mode
	// then perform a single conversion.
	configADC1_ManualCH0(ADC_CH0_POS_SAMPLEA_AN1, 31, 1);
	
	//Configure DAC
	configSPI1();
	configDAC();

	//Configure LED
	CONFIG_LED1();
	
	//Configure LCD
	configControlLCD();      //configure the LCD control lines
	initLCD();  
	
	//Configure timers
	configTimer2();
	configTimer3();             //initialize the LCD

	while (1); //Infinite loop - all work done by Timer3 ISR
}
