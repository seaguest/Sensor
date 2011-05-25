#include "common.h"
#include "interrupt.h"
#include <mrfi.h> 

void timer_wait_first(Status* s){			// wait for message
	s->Counter = (uint16_t) ( DUREE_SLOT*(N_SLOT - s->ID_Beacon) + DUREE_ACTIVE + DUREE_SLEEP);				// delay 1.5ms*MAC 
	Start_Timer();
}

//initialisation for timer A and timer b ; set mode
void Start_Timer(){
	TBCTL=TBSSEL_2 + MC_1;     		  // ACLK = VLO =  12KHz up
	TBCCTL0 = CCIE;                          // TACCR1 interrupt enabled
	TBCCR0 = (uint16_t) N_1MS;				 // delay duty scan
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
	Start_Timer();
}

void timer_wait_message(Status * s){				//after sending beacon , wait for sending message
	s->Counter = (uint16_t) (DUREE_SLOT*(N_SLOT + 1)); // - s->MAC));			// delay TIME_SLOT*(N + 1 - MAC) 
	Start_Timer();
}

void timer_wait_sleep(Status * s){				//after sending message , wait for sleeping
	s->Counter = (uint16_t) DUREE_ACTIVE;					// wait the end of message 
	Start_Timer();
}

void timer_wait_beacon(Status * s){
	s->Counter = (uint16_t) DUREE_SLEEP ; //DUREE_SLOT*s->MAC + DUREE_SLEEP);			//wait the end of sllep
	Start_Timer();
}

