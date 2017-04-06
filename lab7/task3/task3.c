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
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."gc
 *
 * Please maintain this header in its entirety when copying/modifying
 * these files.
 *
 *
 */

#include "pic24_all.h"
#include <stdio.h>


/** \file
Demonstrates a Character LCD using the industry
standard parallel interface in 4-bit mode.
Assumes a 5V LCD; digital-only IO pins are assumed
to be used for the four-bit data bus because they are
5V input tolerant.

Tested with Optrex 24227 and
Optrex DMC-50448N-AAE-AD.
Should work with any LCD that uses the
Hitachi HD44780 LCD controller.
*/

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

/**
 Functions above this line must be redefined for
 your particular PICmicro-to-LCD interface
*/

#define CONFIG_LED1() CONFIG_RB15_AS_DIG_OUTPUT()
#define LED1 _LATB15 //led1 state

inline void CONFIG_SW1() {
	CONFIG_RB14_AS_DIG_INPUT();
	ENABLE_RB14_PULLUP();
	DELAY_US(1);
}

inline void CONFIG_SW2() {
	CONFIG_RB12_AS_DIG_INPUT();
	ENABLE_RB12_PULLUP();
	DELAY_US(1);
}

#define SW1				_RB14		//switch value
#define SW1_PRESSED() 	(SW1==0)	//switch test
#define SW1_RELEASED() 	(SW1==1)	//switch test

#define SW2				_RB12		//switch value
#define SW2_PRESSED() 	(SW2==0)	//switch test
#define SW2_RELEASED() 	(SW2==1)	//switch test

//semaphore variable
volatile uint8_t sw1_pressAndRelease = 0;    //initially cleared
volatile uint8_t sw2_pressAndRelease = 0;    //initially cleared

typedef enum  {
  STATE_RESET = 0,
  STATE_WAIT_FOR_PRESS,
  STATE_WAIT_FOR_RELEASE
} STATE;

volatile STATE sw1_state = STATE_RESET;
volatile STATE sw2_state = STATE_RESET;

//Interrupt Service Routine for Timer3
void _ISRFAST _T3Interrupt (void) {
  if (!sw1_pressAndRelease) {
    //semaphore is cleared, watch for another press & release
    switch (sw1_state) {
      case STATE_WAIT_FOR_PRESS:
        if (SW1_PRESSED()) {
          sw1_state = STATE_WAIT_FOR_RELEASE;
        }
        break;
      case STATE_WAIT_FOR_RELEASE:
        if (SW1_RELEASED()) {
          //have received a complete Press & Release.
          //Set the semaphore
          sw1_pressAndRelease = 1;
          sw1_state = STATE_WAIT_FOR_PRESS;
        }
        break;
      default:
        sw1_state = STATE_WAIT_FOR_PRESS;
    }
  }

  if (!sw2_pressAndRelease) {
    //semaphore is cleared, watch for another press & release
    switch (sw2_state) {
      case STATE_WAIT_FOR_PRESS:
        if (SW2_PRESSED()) {
          sw2_state = STATE_WAIT_FOR_RELEASE;
        }
        break;
      case STATE_WAIT_FOR_RELEASE:
        if (SW2_RELEASED()) {
          //have received a complete Press & Release.
          //Set the semaphore
          sw2_pressAndRelease = 1;
          sw2_state = STATE_WAIT_FOR_PRESS;
        }
        break;
      default:
        sw2_state = STATE_WAIT_FOR_PRESS;
    }
  }

  _T3IF = 0;                 //clear the timer interrupt bit
}

#define ISR_PERIOD     15                // in ms
void  configTimer3(void) {
  //ensure that Timer2,3 configured as separate timers.
  T2CONbits.T32 = 0;     // 32-bit mode off
  //T3CON set like this for documentation purposes.
  //could be replaced by T3CON = 0x0020
  T3CON = T3_OFF |T3_IDLE_CON | T3_GATE_OFF
          | T3_SOURCE_INT
          | T3_PS_1_64 ;  //results in T3CON= 0x0020
  PR3 = msToU16Ticks (ISR_PERIOD, getTimerPrescale(T3CONbits)) - 1;
  TMR3  = 0;                       //clear timer3 value
  _T3IF = 0;                       //clear interrupt flag
  _T3IP = 2;                       //choose a priority
  _T3IE = 1;                       //enable the interrupt
  T3CONbits.TON = 1;               //turn on the timer
}

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

int main (void) {
  configBasic(HELLO_MSG);      // Set up heartbeat, UART, print hello message and diags

  configControlLCD();      //configure the LCD control lines
  initLCD();               //initialize the LCD

  CONFIG_SW1();
  CONFIG_SW2();
  CONFIG_LED1();
  configTimer3(); //use timer3 to periodically sample the switch input

  uint8 count = 0;
  char str[3];
  
  outStringLCD("000");

  while (1) {
	if(sw1_pressAndRelease) { //semaphore received from SW1
		count++; //increment count
		if(count > 10) count = 0; //reset count after 60
   		sprintf(str, "%.3d", count); //format count
  		writeLCD(0x01,0,0,1);  // clear LCD
    	outStringLCD(str); //output count
    	if(count >= 10) LED1 = 1;
		else LED1 = 0;
		sw1_pressAndRelease = 0;
	}
	if(sw2_pressAndRelease) { //semaphore received from SW2
		count = 0; //reset count
		sprintf(str, "%.3d", count); //format count
  		writeLCD(0x01,0,0,1);  // clear LCD
		outStringLCD(str); //output count
		LED1 = 0;
		sw2_pressAndRelease = 0;
	}
  }
}
