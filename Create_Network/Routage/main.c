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
#include "route.h" 


/*
*	global initialisation 
*/
void Init_config(void ){
	WDTCTL = WDTPW + WDTHOLD;
//	WDTCTL = WDT_ADLY_1000;                    // WDT 1s, ACLK, interval timer
//	IE1 |= WDTIE;                             // Enable WDT interrupt

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
	Synchrone_Init(7);			//set MAC

	InitQueue(&FIFO_Send);
	InitQueue(&FIFO_Recieve);
}

/*

// Watchdog Timer interrupt service routine
void Timer_WDT(void);
interrupt(WDT_VECTOR) Timer_WDT(void){
	P1OUT ^= 0x01;                            // Toggle P1.0 using exclusive-OR
}
*/

int main( void )
{
	Init_config();	

	__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

	return 0;
}
