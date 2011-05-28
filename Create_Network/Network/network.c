#include "common.h"
#include <mrfi.h> 
#include <string.h> 
#include "interrupt.h"
#include "uart.h" 
#include "cycle.h" 
#include "stdio.h" 
#include "stdlib.h"
#include "synchrone.h"
#include "fifo.h" 

void Init(){
	WDTCTL = WDTPW + WDTHOLD;

	//set clock 8MHZ
	DCOCTL = 0x0;
	BCSCTL1 = CALBC1_8MHZ;                    // ACLK, Set DCO to 8MHz
	DCOCTL  = CALDCO_8MHZ;
	BCSCTL2 |= DIVS_0;                         // SMCLK = DCO / 1
	BCSCTL3 |= LFXT1S_2;			  // set frequency VLO = 12k

	//set LED
	P1DIR |= 0x03;                            // P1.0 output
	P1OUT |= 0x02;

	Button_Init();
	Synchrone_Init(21);			//set MAC
	InitQueue(&FIFO_Send);
	InitQueue(&FIFO_Recieve);
}


int main( void )
{
	char *s = (char *)malloc(4*sizeof(char));
	s = "ss";
	Init();
	__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

	return 0;
}
