#include "common.h"
#include "interrupt.h"
#include <mrfi.h> 

void timer_synchrone(Status* s){			// wait for message
	if(s->HOST == IS_CREATER){
		s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT + 1)) ;				 
	}else if(s->MAC > s->ID_Beacon){
		s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT - s->MAC + 1) );				 
	}else{
		s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT - s->ID_Beacon + 1) );				 
	}
	Start_Timer(s);
}

void timer_send_beacon(Status* s){			// wait for message
	if(s->MAC > s->ID_Beacon){
		s->Counter = (uint16_t) ( DUREE_SLOT*(s->MAC - s->ID_Beacon ) );				 
	}else{
		s->Counter = (uint16_t) ( DUREE_SLOT*(s->MAC) );				 
	}
	Start_Timer(s);
}

//initialisation for timer A and timer b ; set mode
void Start_Timer(Status* s){
	if(s->state == WAIT_SLEEP){
		TBCTL=TBSSEL_1 + MC_1;     		  
		TBCCTL0 = CCIE;                          // TACCR1 interrupt enabled
		TBCCR0 = (uint16_t)(DUREE_SLEEP * 12) ;	 // DUREE_SLEEP * 1ms		
	}else{
		TBCTL=TBSSEL_2 + MC_1;     		  
		TBCCTL0 = CCIE;                          // TACCR1 interrupt enabled
		TBCCR0 = (uint16_t) N_1MS;		 // delay duty scan
	}
}

void Stop_Timer(){
	TBCCTL0 = ~CCIE;                          // TACCR1 interrupt unenabled
}

//initialisation for button
void Button_Init(){
	P1DIR |=  0x03;
	P1REN |=  0x04;
	P1OUT |=  0x03;
	P1IE  |=  0x04;
}

void Scan_Init(Status * s)				//open the timer of scan ; after the time over ; if not existe a network ;then create one
{
	s->Counter = (uint16_t) DUREE_SCAN;
	Start_Timer(s);
}

void timer_message(Status * s){				//after sending message , wait for sleeping
	s->Counter = (uint16_t) DUREE_ACTIVE;				 
	Start_Timer(s);
}

void timer_sleep(Status * s){
	s->Counter = 1;	//(uint16_t) DUREE_SLEEP ; //DUREE_SLOT*s->MAC + DUREE_SLEEP);			 
	Start_Timer(s);
}

