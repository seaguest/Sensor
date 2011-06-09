
/***************************************************************************************
Copyright (C), 2011-2012, ENSIMAG.TELECOM 
File name	: interrupt.c
Author		: HUANG yongkan & KANJ mahamad
Version		:
Date		: 2011-6-6
Description	: we use the timer interrupts for the  synchronisation , update routage , maintenance of network
		  here are the function of setting timers ,stop timers
Function List	:  
		  void Scan_Init(volatile Status * s);		// initialisation for timer scan 
		  void Button_Init(void );			// initialisation for button
		  void Start_Timer(volatile Status* s);		// timer genaral setting for starting
		  void Stop_Timer(void );			// stop timer
		  void Start_Timer_Surveille(void );		// timer setting for maintenance of network and update route table
		  void timer_send_beacon(volatile Status* s); 	// timer setting for sending beacon
		  void timer_synchrone(volatile Status* s);	// timer setting for synchrone	
		  void timer_message(volatile Status * s);	// timer setting for sending message		 
		  void timer_sleep(volatile Status * s);	// timer setting for sleeping
***************************************************************************************/

#include "common.h"
#include "interrupt.h"
#include <mrfi.h> 
#include "uart.h"
 

/***************************************************************************************
Function	: void Scan_Init(volatile Status * s)		 
Description	: initialisation for timer scan 
		  open the timer of scan ; after the time over ; if not existe a network ;then create one
Calls		:  
Called By	: interrupt(PORT1_VECTOR) Buttopn(void)
Input		: volatile Status * s
Output		: start timer
Return		: void
Others		: 
***************************************************************************************/

void Scan_Init(volatile Status * s)		 
{
	s->Counter = (uint16_t) DUREE_SCAN;
	Start_Timer(s);
}



/***************************************************************************************
Function	: void Button_Init(void ) 
Description	: initialisation for button
Calls		:  
Called By	: void Init_config(void )
Input		: void
Output		: enbale button interrupt
Return		: void
Others		: 
***************************************************************************************/

void Button_Init(void )
{
	P1DIR |=  0x03;
	P1REN |=  0x04;
	P1OUT |=  0x03;
	P1IE  |=  0x04;
}


/***************************************************************************************
Function	: void Start_Timer(volatile Status* s)
Description	: timer genaral setting for starting
		  in sleep mode , we change the source of clock
Calls		:  
Called By	: void timer_send_beacon(volatile Status* s); 
		  void timer_synchrone(volatile Status* s);
		  void timer_message(volatile Status * s);			 
		  void timer_sleep(volatile Status * s);
Input		: volatile Status* s
Output		: enbale timer interrupt and set TBCCR0
Return		: void
Others		: 
***************************************************************************************/

void Start_Timer(volatile Status* s)
{
	TBCTL=TBSSEL_2 + MC_1;     		  
	TBCCTL0 = CCIE;                        
	TBCCR0 = (uint16_t) N_1MS;		 //delay 1ms
/*
	if(s->state == WAIT_SLEEP){			//if it is in mode sleep , we choose the VLO as clock source
		TBCTL=TBSSEL_1 + MC_1;     		  
		TBCCTL0 = CCIE;                          
		TBCCR0 = (uint16_t)(DUREE_SLEEP/2 * 12) ;	//12 means 1ms with VLO=12KHZ 	
	}else{
		if(Clock() == 1){			//if now is TBSSEL_1
			__bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)	
			Set_Clock();
		}
		TBCTL=TBSSEL_2 + MC_1;     		  
		TBCCTL0 = CCIE;                        
		TBCCR0 = (uint16_t) N_1MS;		 //delay 1ms
	}
*/
}


/***************************************************************************************
Function	: void Stop_Timer(void )
Description	: stop timer
Calls		:  
Called By	: void MRFI_RxCompleteISR()
Input		: void
Output		: stop timer
Return		: void
Others		: 
***************************************************************************************/

void Stop_Timer(void )
{
	TBCTL = MC_STOP;
}


/***************************************************************************************
Function	: void Start_Timer_Surveille(void )
Description	: timer setting for maintenance of network and update route table
Calls		:  
Called By	: interrupt(PORT1_VECTOR) Buttopn(void)
		  interrupt(TIMERA0_VECTOR) Timer_Surveille(void)
		  interrupt(TIMERB0_VECTOR) Timer_B0(void)
Input		: void
Output		: start timer A
Return		: void
Others		: 
***************************************************************************************/

void Start_Timer_Surveille(void )
{
	TACTL=TASSEL_1 + MC_1;     		 //choose VLO , which still work in LPM3   
	TACCTL0 = CCIE;                          // TACCR1 interrupt enabled
	TACCR0 = (uint16_t) 12000*3;		 // delay 3s
}


/***************************************************************************************
Function	: void timer_send_beacon(volatile Status* s)
Description	: timer setting for sending beacon
Calls		:  
Called By	: interrupt(TIMERB0_VECTOR) Timer_B0(void)
		  void MRFI_RxCompleteISR()
Input		: volatile Status* s
Output		: Set timer for beacon
Return		: void
Others		: 
***************************************************************************************/

void timer_send_beacon(volatile Status* s)
{			 
	if(s->MAC > s->ID_Beacon){
		s->Counter = (uint16_t) ( DUREE_SLOT*(s->MAC - s->ID_Beacon) );				 
	}else{
		s->Counter = (uint16_t) ( DUREE_SLOT*(s->MAC) );				 
	}
	Start_Timer(s);
}

/***************************************************************************************
Function	: void timer_synchrone(volatile Status* s)
Description	: timer setting for synchronisation
Calls		:  
Called By	: interrupt(TIMERB0_VECTOR) Timer_B0(void)
		  void MRFI_RxCompleteISR()
Input		: volatile Status* s
Output		: Set timer for synchrone
Return		: void
Others		: 
***************************************************************************************/

void timer_synchrone(volatile Status* s)
{			 
	if(s->HOST == IS_CREATER){			//the counter is different which depends the MAC
		s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT + 1)) ;				 
	}else if(s->MAC > s->ID_Beacon){
		s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT - s->MAC + 1) );				 
	}else{
		s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT - s->ID_Beacon + 1) );				 
	}
	Start_Timer(s);
}

/***************************************************************************************
Function	: void timer_synchrone(volatile Status* s)
Description	: initialisation for timer sending message 
		  after sending message , wait for sleeping
Calls		:  
Called By	: interrupt(TIMERB0_VECTOR) Timer_B0(void)
Input		: volatile Status * s
Output		: Set timer for message
Return		: void
Others		: 
***************************************************************************************/

void timer_message(volatile Status * s)
{				
	s->Counter = (uint16_t) DUREE_ACTIVE;				 
	Start_Timer(s);
}


/***************************************************************************************
Function	: void timer_sleep(volatile Status * s)
Description	: initialisation for timer sleep
Calls		:  
Called By	: interrupt(TIMERB0_VECTOR) Timer_B0(void)
Input		: volatile Status * s
Output		: Set timer for sleep
Return		: void
Others		: 
***************************************************************************************/

void timer_sleep(volatile Status * s)
{
	s->Counter = (uint16_t) DUREE_SLEEP ; 			//leave them some time to listen;			 	
	Start_Timer(s);
}

