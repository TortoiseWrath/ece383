#include "pic24_all.h"

/// Red LED
#define CONFIG_RED() CONFIG_RB15_AS_DIG_OUTPUT()
#define RED  _LATB15     //red led state

/// Green LED
#define CONFIG_GREEN() CONFIG_RB14_AS_DIG_OUTPUT()
#define GREEN  _LATB14     //green led state

/// Blue LED
#define CONFIG_BLUE() CONFIG_RB13_AS_DIG_OUTPUT()
#define BLUE  _LATB13     //blue led state

/// Switch1 configuration
inline void CONFIG_SW1()  {
  CONFIG_RB12_AS_DIG_INPUT();     //use RB14 for switch input
  ENABLE_RB12_PULLUP();           //enable the pullup
}
#define SW1              _RB12    //switch state
#define SW1_PRESSED()   (SW1==0)  //switch test
#define SW1_RELEASED()  (SW1==1)  //switch test

/// Switch2 configuration
inline void CONFIG_SW2()  {
  CONFIG_RB11_AS_DIG_INPUT();     //use RB12 for switch input
  ENABLE_RB11_PULLUP();           //enable the pullup
}

#define SW2              _RB11    //switch state
#define SW2_PRESSED()	(SW2==0)  //switch test
#define SW2_RELEASED()	(SW2==1)  //switch test

uint8 bin2gray(uint8 bin){ //Algorithm described in lab instructions.
	uint8 gray = bin >> 1;
	gray ^= bin;
	return gray;
}

uint8 redVal(uint8 bin){ //Get most significant bit of 3-digit binary value
	return bin >> 2;
}

uint8 greenVal(uint8 bin){ //Get middle bit of 3-digit binary value
	return (bin >> 1) & 1;
}

uint8 blueVal(uint8 bin){ //Get least significant bit of 3-digit binary value
	return bin & 1;
}

int main (void) {

	configBasic(HELLO_MSG);      // Set up heartbeat, UART, print hello message and diags

	/** GPIO config ***************************/
	CONFIG_SW1();        //configure switch
	CONFIG_SW2();        //configure switch
	CONFIG_RED();       //configure LED
	CONFIG_GREEN();	   //configure LED
	CONFIG_BLUE();			//configure LED
	DELAY_US(1);         //give pullups a little time
	
	uint8 binaryCode = 0;
	uint8 grayCode; //CHANGE TO CHAR
	
	while (1) {
		if(SW1_RELEASED() && SW2_RELEASED()) {
			RED = 1;
			GREEN = 1;
			BLUE = 1;
			outString("All LEDs ON");
		}
		else if (SW1_RELEASED() && SW2_PRESSED()) {
			outString("Binary code cycle");
			binaryCode++;
			if(binaryCode == 8) binaryCode = 0;
			RED = redVal(binaryCode);
			GREEN = greenVal(binaryCode);
			BLUE = blueVal(binaryCode);
			DELAY_MS(500);
		}
		else if (SW1_PRESSED() && SW2_RELEASED()) {
			outString("Gray code cycle");
			binaryCode++;
			if(binaryCode == 8) binaryCode = 0;
			grayCode = bin2gray(binaryCode);
			RED = redVal(grayCode);
			GREEN = greenVal(grayCode);
			BLUE = blueVal(grayCode);
			DELAY_MS(500);
		}
		else {
			outString("Blink");
			RED = !RED;
			GREEN = !GREEN;
			BLUE = !BLUE;
			DELAY_MS(50); //10 Hz = 100 ms/cycle = 50 ms/toggle
		}
		DELAY_MS(DEBOUNCE_DLY);
		doHeartbeat();
	}
	return 0;
}