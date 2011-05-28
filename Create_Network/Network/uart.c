#include "common.h"
#include "uart.h"
#include <mrfi.h> 

void Uart_Init(){
	P3SEL    |= 0x30;     	// P3.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1  = UCSSEL_2; 	// SMCLK
	UCA0BR0   = 0x41;     	// 9600 from 8Mhz
	UCA0BR1   = 0x3;
	UCA0MCTL  = UCBRS_2;                     
	UCA0CTL1 &= ~UCSWRST; 	// Initialize USCI state machine
//	IE2      |= UCA0RXIE; 	// Enable USCI_A0 RX interrupt
}  

// taken from an earlier version of bsp_board.c
void TXString(char* string, int length)
{
	int pointer;
	for( pointer = 0; pointer < length; pointer++)
	{
		UCA0TXBUF = string[pointer];
		while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
	}
}

