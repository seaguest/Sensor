#include "common.h"
#include "uart.h"
#include <mrfi.h> 
#include "fifo.h" 
#include "route.h" 


extern Status etat;
uint8_t UART_MODE = 0 , time = 0;
char dest[3]="";

void Uart_Init(){
	P3SEL    |= 0x30;     	// P3.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1  = UCSSEL_2; 	// SMCLK
	UCA0BR0   = 0x41;     	// 9600 from 8Mhz
	UCA0BR1   = 0x3;
	UCA0MCTL  = UCBRS_2;                     
	UCA0CTL1 &= ~UCSWRST; 	// Initialize USCI state machine
	IE2      |= UCA0RXIE; 	// Enable USCI_A0 RX interrupt
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


void USCI0RX_ISR(void);
interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
{
	char rx = UCA0RXBUF;
	if(rx == 27){		//ESC
		UART_MODE = 0;
	}

	switch (UART_MODE){
		case 0:
			if(rx == 's'){
				print("\n\r");
				Show_Online(&etat);	// show people online 
				UART_MODE = 1;
			}else if(rx == 'r'){		//show router table
				print("\n\r");
				Show_router(&etat);
				UART_MODE = 1;
			}else if(rx == 'c'){		//continue to talk
				print("\n\r");
				UART_MODE = 2;
			}else if(rx == 27){		//help info
				print("\n\r");
				print(" bienvenu, vous puvez chat avec d'autre \n\r");
				print(" command help: \n\r");
				print(" s: show who is on line ,and choose one XX \n\r");
				print(" r: show router table \n\r");
				print(" c: start to chat \n\r");
			}else{
				print("\n\r");
				print("please enter s or r or ESC\n\r");
			}					
			break;
		case 1:
			if(rx>='0'&&rx<='9'){
				print("\n\r");
				etat.Dst = etat.Dst*10 + rx - '0';
				dest[time] = rx;
				time++;
				if(time == 2){
					if(etat.Route_table[etat.Dst-1].Dst[3]==0){
						print("Sorry, not reachable ,again \n\r");
						time = 0;
						etat.Dst = 0;
					}else{
						UART_MODE = 2;
						print("you choose \n\r");
						print(dest);
						print("\n\r");
						print("OK ,you can enter c to start \n\r");
					}		
				}
			}else{
				print("\n\r");
				print("please enter number \n\r");
			}
			break;
		case 2:
			EnQueue(&FIFO_Send,rx);
			break;
	}

	if(rx == '\r'){
		P1OUT ^= 0x02; 
		print("\n\r");
	}
	TXString(&rx, 1);
}

