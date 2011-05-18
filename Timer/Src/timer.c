#include "io.h"

void TimerA_Init(void)
{
	TACTL = TASSEL_1 + ID0 + TACLR ; //ACLK=32768Hz  
	CCTL0 = CCIE;
	CCR0 = 40950;   // (1/32768)*40950 =1000 ms 
	TACTL |= MC0;  
}

int main(void){
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	P1DIR |= 0x03;                            // P1.0 output
	P1OUT = 0x02;
	TACCTL0 = CCIE;                           // TACCR0 interrupt enabled
	TimerA_Init();

//	TACCR0 = 500;

	__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
}

// Timer A0 interrupt service routine
__attribute__((interrupt(TIMERA0_VECTOR)))
void Timer_A (void){
	P1OUT ^= 0x01;        
	TimerA_Init();                    
//	TACCR0 += 500000;                           
}



