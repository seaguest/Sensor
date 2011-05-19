#include "io.h"

void TimerA0_Init(void)
{
	TACCTL0 = CCIE;                         // TACCR0 interrupt enabled
	TACCR0 = 12000;				// delay 1.5ms
	TACTL = TASSEL_1 + MC_1;     		  // ACLK = VLO =  12KHz upmode
}

void TimerB0_Init(void)
{
	TBCCTL0 = CCIE;                         // TACCR1 interrupt enabled
	TBCCR0 = 6000;				// delay 1.5ms
	TBCTL = TBSSEL_1 + MC_1;     		  // ACLK = VLO =  12KHz upmode
}

int main(void){
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	
	//set the clock
	BCSCTL3 |= LFXT1S_2;			  // set frequency VLO = 12k

	P1DIR |= 0x03;                            // P1.0 output
	P1OUT = 0x02;
	TimerA0_Init();	
	TimerB0_Init();

	__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
}

// Timer A0 interrupt service routine
__attribute__((interrupt(TIMERA0_VECTOR)))
void Timer_A0 (void){
	P1OUT ^= 0x01;   
}

// Timer B0 interrupt service routine
__attribute__((interrupt(TIMERB0_VECTOR)))
void Timer_B0 (void){
	P1OUT ^= 0x02;   
}


