#include "pic24_all.h"
#if __PIC24HJ128GP502__
	#define LED1 _LATA0		//MicroStick II definitions
	#define CONFIG_LED1() CONFIG_RA0_AS_DIG_OUTPUT()
#endif

int main(void) {
	CONFIG_LED1();
	LED1=0;
	while(1) {				//Infinite while loop
		LED1 = !LED1;		//Toggle LED1
		DELAY_MS(100);		//Delay 100ms
	}
	return 0;
}