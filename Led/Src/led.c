#include "io.h"

void delay(){
	volatile unsigned int i = 60000;
	while(i--);
}

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                	  
	P1DIR |= 0x03;                           	  
	P1OUT  = 0x02;	

	while(1){
		P1OUT ^= 0x03;                          
		delay();
	}
}

