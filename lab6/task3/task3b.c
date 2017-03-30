#include "pic24_all.h"
#if __PIC24HJ128GP502__
	#define LED1 _LATA0			//MicroStick II definitions
	#define CONFIG_LED1() CONFIG_RA0_AS_DIG_OUTPUT()
#endif

int main(void) {
	CONFIG_LED1();
	LED1=0;
	uint8 i;
	uint16 time; 				//up to 65535 deciseconds
	while(1) {					//Infinite while loop
		//5 seconds / 100 ms (10 toggles/sec) = 50 iterations
		for(i = 0; i < 50; i++){
			LED1 = !LED1;		//Toggle LED1
			DELAY_MS(100);		//Delay 100ms
			time += 1;			//Add 1ds to elapsed time
		}
		//5 seconds / 200 ms (5 toggles/sec) = 25 iterations
		for(i = 0; i < 25; i++){
			LED1 = !LED1;		//Toggle LED1
			DELAY_MS(200);		//Delay 200ms
			time += 2;			//Add 2ds to elapsed time
		}
	}
	return 0;
}