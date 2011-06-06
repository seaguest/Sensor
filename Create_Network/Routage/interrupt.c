#include "common.h"
#include "interrupt.h"
#include <mrfi.h> 
#include "uart.h"

/*
*	we use the timer interrupts for the  synchronisation , update routage , maintenance of network
*	here are the function of setting timers ,stop timers
*/

/*
*	find out which clock is used
*/
uint8_t Clock(void ){
	uint16_t c, tmp = (3<<8);
	c = (TBCTL & tmp)>>8 ;
	return c;
}


/*
*	timer setting for synchronisation
*/
void timer_synchrone(Status* s){			 
	if(s->HOST == IS_CREATER){			//the counter is different which depends the MAC
		s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT + 1)) ;				 
	}else if(s->MAC > s->ID_Beacon){
		s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT - s->MAC + 1) );				 
	}else{
		s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT - s->ID_Beacon + 1) );				 
	}
	Start_Timer(s);
}

/*
*	timer setting for sending beacon
*/
void timer_send_beacon(Status* s){			 
	if(s->MAC > s->ID_Beacon){
		s->Counter = (uint16_t) ( DUREE_SLOT*(s->MAC - s->ID_Beacon) );				 
	}else{
		s->Counter = (uint16_t) ( DUREE_SLOT*(s->MAC) );				 
	}
	Start_Timer(s);
}

/*
*	timer genaral setting for starting
*	in sleep mode , we change the source of clock
*/
void Start_Timer(Status* s){
	if(s->state == WAIT_SLEEP){			//if it is in mode sleep , we choose the VLO as clock source
		TBCTL=TBSSEL_1 + MC_1;     		  
		TBCCTL0 = CCIE;                          
		TBCCR0 = (uint16_t)(DUREE_SLEEP/2 * 12) ;	//12 means 1ms with VLO=12KHZ 	
	}else{
		if(Clock() == 1){			//if now is TBSSEL_1
			__bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)	
		}
		TBCTL=TBSSEL_2 + MC_1;     		  
		TBCCTL0 = CCIE;                        
		TBCCR0 = (uint16_t) N_1MS;		 //delay 1ms
	}
}

/*
*	timer setting for maintenance of network and update route table
*/
void Start_Timer_Surveille(void ){
	TACTL=TASSEL_1 + MC_1;     		 //choose VLO , which still work in LPM3   
	TACCTL0 = CCIE;                          // TACCR1 interrupt enabled
	TACCR0 = (uint16_t) 12000*3;		 // delay 3s
}

/*
*	stop timer
*/
void Stop_Timer(void ){
	TBCTL = MC_STOP;
	TBCCTL0 = ~CCIE;                        
	TBCCR0 = 0;
}

/*
*	initialisation for button
*/
void Button_Init(void ){
	P1DIR |=  0x03;
	P1REN |=  0x04;
	P1OUT |=  0x03;
	P1IE  |=  0x04;
}

/*
*	initialisation for timer scan 
*/
void Scan_Init(Status * s)				//open the timer of scan ; after the time over ; if not existe a network ;then create one
{
	s->Counter = (uint16_t) DUREE_SCAN;
	Start_Timer(s);
}

/*
*	initialisation for timer sending message 
*/
void timer_message(Status * s){				//after sending message , wait for sleeping
	s->Counter = (uint16_t) DUREE_ACTIVE;				 
	Start_Timer(s);
}

/*
*	initialisation for timer sleep
*/
void timer_sleep(Status * s){
	if(s->HOST == IS_CREATER){			//the counter is different which depends the MAC
		s->Counter = 2; 	 
	}else{
		s->Counter = 2; 			//leave them some time to listen;			 
	}
	Start_Timer(s);
}

