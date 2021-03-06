
/***************************************************************************************
Copyright (C), 2011-2012, ENSIMAG.TELECOM 
File name	: uart.c
Author		: HUANG yongkan & KANJ mahamad
Version		:
Date		: 2011-6-6
Description	: we use uart as a interface of iteraction with machine
		  uart recieve what we type and deals with the commands
Function List	:  
		  void Uart_Init(void );				// initialisation de uart
		  void TXString(char* string, int length);		// send message in uart
		  void print(char *s);					// print a string
		  void print_8b(uint8_t u );				// print a number uint8_t
		  void print_16b(uint16_t u );				// print a number uint16_t
		  void print_32b(uint32_t u );				// print a number uint32_t
***************************************************************************************/

#include "common.h"
#include "uart.h"
#include <mrfi.h> 
#include "fifo.h" 
#include "route.h" 
#include "string.h" 
#include "synchrone.h" 

//define the variable global and variable from other files
extern Status etat;	
volatile uint8_t UART_MODE = 0 , time = 0;
char dest[2]="";


/***************************************************************************************
Function	: uint8_t Clock(void )
Description	: find out which clock is used
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: void
Output		: 
Return		: return the value of TBCTL
Others		: 
***************************************************************************************/

uint8_t Clock(void )
{
	uint16_t c, tmp = (3<<8);
	c = (TBCTL & tmp)>>8 ;
	return c;
}
 
/***************************************************************************************
Function	: void print(char *s)
Description	: print a string in the minicom 
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: char *s
Output		: print the string
Return		: void
Others		: 
***************************************************************************************/

void print(char *s)
{
	TXString(s, strlen(s));
}

/***************************************************************************************
Function	: void print_8b(uint8_t u)
Description	: print a number uint8_t
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: uint8_t u
Output		: print the uint8_t
Return		: void
Others		: 
***************************************************************************************/

void print_8b(uint8_t u)
{
	char o[4] = "";
	if(u < 100){
		o[0] = u/10 + '0' ;
		o[1] = u%10 + '0' ;
		o[2] = 0 ;
		TXString(o, 3);
	}else{
		o[0] = u/100 + '0' ;
		o[1] = u%100/10 + '0' ;
		o[2] = u%10 + '0' ;
		o[3] = 0 ;
		TXString(o, 4);
	}
}


/***************************************************************************************
Function	: void print_16b(uint16_t u)
Description	: print a number uint16_t
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: uint16_t u
Output		: print the uint16_t
Return		: void
Others		: 
***************************************************************************************/

void print_16b(uint16_t u )
{
	char o[6] = "";
	o[0] = u/10000 + '0' ;
	o[1] = u%10000/1000 + '0' ;
	o[2] = u%1000/100 + '0' ;
	o[3] = u%100/10 + '0' ;
	o[4] = u%10 + '0' ;
	o[5] = 0 ;
	TXString(o, 6);
	print("\n\r");
}


/***************************************************************************************
Function	: void print_32b(uint32_t u)
Description	: print a number uint32_t
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: uint32_t u
Output		: print the uint32_t
Return		: void
Others		: 
***************************************************************************************/

void print_32b(uint32_t u )
{
	char o[11] = "";
	o[0] = u/1000000000 + '0' ;
	o[1] = u%1000000000/100000000 + '0' ;
	o[2] = u%100000000/10000000 + '0' ;
	o[3] = u%10000000/1000000 + '0' ;
	o[4] = u%1000000/100000 + '0' ;
	o[5] = u%100000/10000 + '0' ;
	o[6] = u%10000/1000 + '0' ;
	o[7] = u%1000/100 + '0' ;
	o[8] = u%100/10 + '0' ;
	o[9] = u%10 + '0' ;
	o[10] = 0 ;
	TXString(o, 11);
	print("\n\r");
}


/***************************************************************************************
Function	: void Uart_Init(void)
Description	: initialisation of uart
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: void
Output		: set the uart
Return		: void
Others		: 
***************************************************************************************/

void Uart_Init(void)
{
	P3SEL    |= 0x30;     	// P3.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1  = UCSSEL_2; 	// SMCLK
	UCA0BR0   = 0x41;     	// 9600 from 8Mhz
	UCA0BR1   = 0x3;
	UCA0MCTL  = UCBRS_2;                     
	UCA0CTL1 &= ~UCSWRST; 	// Initialize USCI state machine
	IE2      |= UCA0RXIE; 	// Enable USCI_A0 RX interrupt
}  


/***************************************************************************************
Function	: void TXString(char* string, int length)
Description	: send the string to the serial interface 
Calls		:  
Called By	: 
Input		: void
Output		: print string
Return		: void
Others		: 
***************************************************************************************/

void TXString(char* string, int length)
{
	int pointer;
	for( pointer = 0; pointer < length; pointer++)
	{
		UCA0TXBUF = string[pointer];
		while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
	}
}

/***************************************************************************************
Function	: void TXString(char* string, int length)
Description	: interruption of uart
		  iteraction with the command typed in
Calls		:  
Called By	: 
Input		: void
Output		: 
Return		: void
Others		: 
***************************************************************************************/

void USCI0RX_ISR(void);
interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
{
	char rx = UCA0RXBUF;
	uint16_t clock ,tmp = ( 3<<8 );

	if(rx == 27){		//ESC
		UART_MODE = 0;
		time = 0;
		etat.Dst = 0;
		InitQueue(&etat.FIFO_Send);
		InitQueue(&etat.FIFO_Recieve);
	}

	switch (UART_MODE){
		case 0:
			if(rx == 'o'){			// show people online 
				print("\n\r");
				Show_Online(&etat);	
				print("choose one !\n\r");
				UART_MODE = 1;
			}else if(rx == 'v'){			// show voisin 
				print("\n\r");
				Show_voisin(&etat);
			}else if(rx == 'r'){		//show router table
				print("\n\r");
				Tidy_table(&etat);		// clear the dirt data
				Show_router(&etat);
			}else if(rx == 'i'){		//print information
				print("\n\r");
				print("ID_Network :");
				print_8b(etat.ID_Network);
				print("\n\r");

				print("MAC        :");
				print_8b(etat.MAC);
				print("\n\r");

				print("ID_Beacon  :");
				print_8b(etat.ID_Beacon);
				print("\n\r");

				print("HOST       :");
				if(etat.HOST == IS_CREATER){
					print("IS_CREATER");
				}else if(etat.HOST == IS_NOT_CREATER){
					print("IS_NOT_CREATER");
				}
				print("\n\r");

				print("synchrone  :");
				print_8b(etat.synchrone);
				print("\n\r");

				print("state      :");
				switch(etat.state){
					case WAIT_BEACON:
						print("WAIT BEACON");
						break;
					case WAIT_SYNCHRONE:
						print("WAIT_SYNCHRONE");
						break;
					case WAIT_MESSAGE:
						print("WAIT_MESSAGE");
						break;
					case WAIT_SLEEP:
						print("WAIT_SLEEP");
						break;
				}
				print("\n\r");

				print("clock      :");
				clock = (TBCTL & tmp)>>8 ;
				if(clock == 1){		//vlo
					print("VLO 12KHZ");
				}else if(clock == 2){	//dco
					print("DCO 8MHZ");
				}
				print("\n\r");

				print("Counter    :");
				print_16b(etat.Counter);

				print("Voisin     :");
				print_32b(etat.Voisin);
			}else if(rx == 27){		//help info
				print("\n\r");
				print("command: \n\r");
				print("o  : who is on line \n\r");
				print("v  : voisin \n\r");
				print("r  : router table \n\r");
				print("i  : sysinfo \n\r");
				print("ESC: help \n\r");
			}else{
				print("\n\r");
				print("only s or r or ESC \n\r");
			}			
			break;
		case 1:
			if(rx>='0'&&rx<='9'){
				print("\n\r");
				etat.Dst = etat.Dst*10 + rx - '0';
				dest[time] = rx;
				time++;
				if(time == 2){
					if(etat.Route_table[etat.Dst-1].Dst==0){
						print("Sorry, not reachable ,again \n\r");
						time = 0;
						etat.Dst = 0;
					}else{
						UART_MODE = 2;
						print(dest);
						print("is choosen \n\r");
						print("OK ,start talking...  \n\r");
					}		
				}
			}else{
				print("\n\r");
				print("please enter number \n\r");
			}
			break;
		case 2:
			EnQueue(&etat.FIFO_Send,rx);
			break;
	}
 
	if(rx == '\r'){
		P1OUT ^= 0x02; 
		print("\n\r");
	}
	TXString(&rx, 1);
}

