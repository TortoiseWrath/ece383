#include "pic24_all.h"

void main(void){

	uint16 check_val = 0xF508;
	uint8 ones_count = 0;
	uint8 first_one = 16; //Return 16 if no 1 bits found.
	
	uint8 pos;
	
	for(pos = 0; pos < 16; check_val >>= 1, pos++) {
		if(check_val & 1 && !ones_count++) { //bitwise AND with 0x0001 to find LSB; if LSB is 1, increment ones_count
			// body entered where LSB is 1 and ones_count previously equaled 0 (no 1 bits had previously been found)
			first_one = pos;
		}
	}
	
	return;
}