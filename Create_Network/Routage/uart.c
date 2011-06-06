#include "common.h"
#include "uart.h"
#include <mrfi.h> 
#include "fifo.h" 
#include "route.h" 
#include "string.h" 
#include "synchrone.h" 


/*
*	we use the uart to communicate with the MCU
*	setting of the uart and iteraction with the command 
*/

extern Status etat;
uint8_t UART_MODE = 0 , time = 0;
char dest[3]="";

/*
*	print a string in the minicom 
*/
void print(char *s){
	TXString(s, strlen(s));
}

/*
*	initialisation of uart
*/
void Uart_Init(void){
	P3SEL    |= 0x30;     	// P3.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1  = UCSSEL_2; 	// SMCLK
	UCA0BR0   = 0x41;     	// 9600 from 8Mhz
	UCA0BR1   = 0x3;
	UCA0MCTL  = UCBRS_2;                     
	UCA0CTL1 &= ~UCSWRST; 	// Initialize USCI state machine
	IE2      |= UCA0RXIE; 	// Enable USCI_A0 RX interrupt
}  

/*
*	send the string to the serial interface 
*/
void TXString(char* string, int length)
{
	int pointer;
	for( pointer = 0; pointer < length; pointer++)
	{
		UCA0TXBUF = string[pointer];
		while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
	}
}

/*
*	interruption of uart
*	iteraction with the command typed in
*/
void USCI0RX_ISR(void);
interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
{
	char rx = UCA0RXBUF;
	char info[3] = "";
	char cnt[6] = "";

	uint16_t clock ,tmp = ( 3<<8 );

	if(rx == 27){		//ESC
		UART_MODE = 0;
		time = 0;
		etat.Dst = 0;
		CleanQueue(&FIFO_Send);
		CleanQueue(&FIFO_Recieve);
		InitQueue(&FIFO_Send);
		InitQueue(&FIFO_Recieve);
	}

	switch (UART_MODE){
		case 0:
			if(rx == 'o'){			// show people online 
				print("\n\r");
				Show_Online(&etat);	
				UART_MODE = 1;
			}else if(rx == 'r'){		//show router table
				print("\n\r");
				Show_router(&etat);
			}else if(rx == 'i'){		//print information
				print("\n\r");
				print("ID_Network :");
				info[0] = etat.ID_Network/10 + '0' ;
				info[1] = etat.ID_Network%10 + '0' ;
				print(info);
				print("\n\r");

				print("MAC        :");
				info[0] = etat.MAC/10 + '0' ;
				info[1] = etat.MAC%10 + '0' ;
				print(info);
				print("\n\r");

				print("ID_Beacon  :");
				info[0] = etat.ID_Beacon/10 + '0' ;
				info[1] = etat.ID_Beacon%10 + '0' ;
				print(info);
				print("\n\r");

				print("HOST       :");
				if(etat.HOST == IS_CREATER){
					print("IS_CREATER\n\r");
				}else if(etat.HOST == IS_NOT_CREATER){
					print("IS_NOT_CREATER\n\r");
				}

				print("synchrone  :");
				info[0] = etat.synchrone%10 + '0' ;
				info[1] = 0 ;
				print(info);
				print("\n\r");

				print("state      :");
				switch(etat.state){
					case WAIT_BEACON:
						print("WAIT BEACON \n\r");
						break;
					case WAIT_SYNCHRONE:
						print("WAIT_SYNCHRONE \n\r");
						break;
					case WAIT_MESSAGE:
						print("WAIT_MESSAGE \n\r");
						break;
					case WAIT_SLEEP:
						print("WAIT_SLEEP \n\r");
						break;
				}

				print("clock      :");
				clock = (TBCTL & tmp)>>8 ;
				if(clock == 1){		//vlo
					print("VLO 12KHZ \n\r");
				}else if(clock == 2){	//dco
					print("DCO 8MHZ \n\r");
				}

				print("Counter    :");
				cnt[0] = etat.Counter/10000 + '0' ;
				cnt[1] = etat.Counter%10000/1000 + '0' ;
				cnt[2] = etat.Counter%1000/100 + '0' ;
				cnt[3] = etat.Counter%100/10 + '0' ;
				cnt[4] = etat.Counter%10 + '0' ;
				cnt[5] = 0 ;
				print(cnt);
				print("\n\r");
			}else if(rx == 27){		//help info
				print("\n\r");
				print("command: \n\r");
				print("o  : who is on line \n\r");
				print("r  : router table \n\r");
				print("i  : sysinfo \n\r");
				print("ESC: help \n\r");
			}else{
				print("\n\r");
				print("only s or r or ESC\n\r");
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
						print(dest);
						print(" choosen \n\r");
						print("OK ,start talking... \n\r");
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

