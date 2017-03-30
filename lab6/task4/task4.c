#include "pic24_all.h"

/// LED1
#define CONFIG_LED1() CONFIG_RB15_AS_DIG_OUTPUT()
#define LED1  _RB15     //led1 state

/// LED2
#define CONFIG_LED2() CONFIG_RB2_AS_DIG_OUTPUT()
#define LED2  _RB2	  //led2 state

/// Switch1 configuration
inline void CONFIG_SW1()  {
  CONFIG_RB14_AS_DIG_INPUT();     //use RB14 for switch input
  ENABLE_RB14_PULLUP();           //enable the pullup
}
#define SW1              _RB12    //switch state
#define SW1_PRESSED()   (SW1==0)  //switch test
#define SW1_RELEASED()  (SW1==1)  //switch test

/// Switch2 configuration
inline void CONFIG_SW2()  {
  CONFIG_RB12_AS_DIG_INPUT();     //use RB12 for switch input
  ENABLE_RB12_PULLUP();           //enable the pullup
}

#define SW2              _RB11    //switch state
#define SW2_PRESSED()	(SW2==0)  //switch test
#define SW2_RELEASED()	(SW2==1)  //switch test


typedef enum  {
  STATE_RESET = 0,
  STATE_STEP_1_WAIT_ON_FOR_PRESS,
  STATE_STEP_1_WAIT_ON_FOR_RELEASE,
  STATE_STEP_1_WAIT_OFF_FOR_PRESS,
  STATE_STEP_1_WAIT_OFF_FOR_RELEASE,
  STATE_STEP_2_BLINK_TWICE,
  STATE_STEP_3_WAIT_ON_FOR_PRESS,
  STATE_STEP_3_WAIT_ON_FOR_RELEASE,
  STATE_STEP_4_WAIT_FOR_PRESS1,
  STATE_STEP_4_BLINK1,
  STATE_STEP_4_WAIT_FOR_PRESS2,
  STATE_STEP_4_BLINK2,
  STATE_STEP_5_BLINK_WAIT_FOR_PRESS,
  STATE_STEP_5_BLINK_WAIT_FOR_RELEASE
} STATE;


STATE e_LastState = STATE_RESET;
//print debug message for state when it changes
void printNewState (STATE e_currentState) {
	if (e_LastState != e_currentState) {
		switch (e_currentState) {
			case STATE_STEP_1_WAIT_ON_FOR_PRESS:
				outString("STATE_STEP_1_WAIT_ON_FOR_PRESS - LED on, waiting for SW1 press");
				break;
			case STATE_STEP_1_WAIT_ON_FOR_RELEASE:
				outString("STATE_STEP_1_WAIT_ON_FOR_RELEASE - LED on, waiting for SW1 release");
				break;
			case STATE_STEP_1_WAIT_OFF_FOR_PRESS:
				outString("STATE_STEP_1_WAIT_OFF_FOR_PRESS - LED off, waiting for SW1 press");
				break;
			case STATE_STEP_1_WAIT_OFF_FOR_RELEASE:
				outString("STATE_STEP_1_WAIT_OFF_FOR_RELEASE - LED off, waiting for SW1 release");
				break;
			case STATE_STEP_2_BLINK_TWICE:
				outString("STATE_STEP_2_BLINK_TWICE - Blinking LED twice");
				break;
			case STATE_STEP_3_WAIT_ON_FOR_PRESS:
				outString("STATE_STEP_3_WAIT_ON_FOR_PRESS - LED on, waiting for SW1 press");
				break;
			case STATE_STEP_3_WAIT_ON_FOR_RELEASE:
				outString("STATE_STEP_3_WAIT_ON_FOR_RELEASE - LED on, waiting for SW1 release");
				break;
			case STATE_STEP_4_WAIT_FOR_PRESS1:
				outString("STATE_STEP_4_WAIT_FOR_PRESS1 - Waiting for SW1 press");
				break;
			case STATE_STEP_4_BLINK1:
				outString("STATE_STEP_4_BLINK1 - Blinking LED2 at 5 Hz while SW1 pressed");
				break;
			case STATE_STEP_4_WAIT_FOR_PRESS2:
				outString("STATE_STEP_4_WAIT_FOR_PRESS2 - Waiting for SW1 press");
				break;
			case STATE_STEP_4_BLINK2:
				outString("STATE_STEP_4_BLINK2 - Blinking LED2 at 5 Hz while SW1 pressed");
				break;
			case STATE_STEP_5_BLINK_WAIT_FOR_PRESS:
				outString("STATE_STEP_5_BLINK_WAIT_FOR_PRESS - Blinking LED2 at 10 Hz, waiting for SW2 press");
				break;
			case STATE_STEP_5_BLINK_WAIT_FOR_RELEASE:
				outString("STATE_STEP_5_BLINK_WAIT_FOR_RELEASE - Blinking LED2 at 10 Hz, waiting for SW2 release");
				break;
			default:
				outString("Default case - this should not happen");
		}
	}
	e_LastState = e_currentState;  //remember last state
} 

int main (void) {
	STATE e_mystate;

	configBasic(HELLO_MSG);      // Set up heartbeat, UART, print hello message and diags

	/** GPIO config ***************************/
	CONFIG_SW1();        //configure switch
	CONFIG_SW2();        //configure switch
	CONFIG_LED1();       //configure LED
	CONFIG_LED2();	   //configure LED
	DELAY_US(1);         //give pullups a little time
	
	
	e_mystate = STATE_STEP_1_WAIT_ON_FOR_PRESS;

	while (1) {
		printNewState(e_mystate);  //debug message when state changes
		switch (e_mystate) {
			case STATE_STEP_1_WAIT_ON_FOR_PRESS:
				LED1 = 1;
				LED2 = 0;
				if(SW1_PRESSED()) e_mystate = STATE_STEP_1_WAIT_ON_FOR_RELEASE;
				break;
			case STATE_STEP_1_WAIT_ON_FOR_RELEASE:
				if(SW1_RELEASED()) e_mystate = STATE_STEP_1_WAIT_OFF_FOR_PRESS;
				break;
			case STATE_STEP_1_WAIT_OFF_FOR_PRESS:
				LED1 = 0;
				if(SW1_PRESSED()) e_mystate = STATE_STEP_1_WAIT_OFF_FOR_RELEASE;
				break;
			case STATE_STEP_1_WAIT_OFF_FOR_RELEASE:
				if(SW1_RELEASED()) e_mystate = STATE_STEP_2_BLINK_TWICE;
				break;
			case STATE_STEP_2_BLINK_TWICE:
				LED1 = 1;
				DELAY_MS(250);
				LED1 = 0;
				DELAY_MS(250);
				LED1 = 1;
				DELAY_MS(250);
				LED1 = 0;
				DELAY_MS(250);
				if(SW1_RELEASED()) e_mystate = STATE_STEP_3_WAIT_ON_FOR_PRESS;
				break;
			case STATE_STEP_3_WAIT_ON_FOR_PRESS:
				LED1 = 1;
				if(SW1_PRESSED()) e_mystate = STATE_STEP_3_WAIT_ON_FOR_RELEASE;
				break;
			case STATE_STEP_3_WAIT_ON_FOR_RELEASE:
				if(SW1_RELEASED()) {
					if(SW2_PRESSED()) e_mystate = STATE_STEP_4_WAIT_FOR_PRESS1;
					else e_mystate = STATE_STEP_1_WAIT_ON_FOR_PRESS;
				}
				break;
			case STATE_STEP_4_WAIT_FOR_PRESS1:
				if(SW1_PRESSED()) e_mystate = STATE_STEP_4_BLINK1;
				break;
			case STATE_STEP_4_BLINK1:
				LED2 = !LED2;
				DELAY_MS(100);
				if(SW1_RELEASED()) e_mystate = STATE_STEP_4_WAIT_FOR_PRESS2;
				break;
			case STATE_STEP_4_WAIT_FOR_PRESS2:
				if(SW1_PRESSED()) e_mystate = STATE_STEP_4_BLINK2;
				break;
			case STATE_STEP_4_BLINK2:
				LED2 = !LED2;
				DELAY_MS(100);
				if(SW1_RELEASED()) e_mystate = STATE_STEP_5_BLINK_WAIT_FOR_PRESS;
				break;
			case STATE_STEP_5_BLINK_WAIT_FOR_PRESS:
				LED2 = !LED2;
				DELAY_MS(50);
				if(SW2_PRESSED()) e_mystate = STATE_STEP_5_BLINK_WAIT_FOR_RELEASE;
				break;
			case STATE_STEP_5_BLINK_WAIT_FOR_RELEASE:
				LED2 = !LED2;
				DELAY_MS(50);
				if(SW2_RELEASED()) e_mystate = STATE_STEP_1_WAIT_ON_FOR_PRESS;
				break;
			default:
				e_mystate = STATE_STEP_1_WAIT_ON_FOR_PRESS;
		}
		DELAY_MS(DEBOUNCE_DLY);
	}
	return 0;
}