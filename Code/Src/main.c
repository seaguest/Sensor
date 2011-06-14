
/***************************************************************************************
Copyright (C), 2011-2012, ENSIMAG.TELECOM 
File name	: main.c
Author		: HUANG yongkan & KANJ mahamad
Version		:
Date		: 2011-6-6
Description	: main
Function List	:  
		  void Init_config(void )
		  int main( void )
***************************************************************************************/
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
#include "uart.h" 

//define the variable frome the other files
extern Status etat;


/***************************************************************************************
Function	: void Init_config(void )
Description	: global initialisation 
Calls		:  
Called By	: main()
Input		: void
Output		: 
Return		: void
Others		: 
***************************************************************************************/

void Init_config(void )
{
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
	Synchrone_Init(9);			//set MAC
}

/***************************************************************************************
Function	: int main( void )
Description	: main
Calls		:  
Called By	: 
Input		: void
Output		: 
Return		: void
Others		: 
***************************************************************************************/
int main( void )
{
	Init_config();	

	__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt

	return 0;
}
