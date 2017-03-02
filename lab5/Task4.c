#include "pic24_all.h"

void main(void){
	uint8 count = 3;
	uint16 x = 1;
	uint16 y = 3;

	while (count){
		if(x-y == 0) y++;
		if(x < y) x += 2;
		count--;
	}
}