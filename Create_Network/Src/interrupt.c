#include "common.h"
#include "interrupt.h"
#include <mrfi.h> 

void Timer_synchrone(){
	TACTL=TASSEL_2 + MC_1;     	 
	TACCTL0 = CCIE;                         
	TACCR0 = N_1MS*5;	//5ms				
}

void wait_beacon_first(Status* s){			//synchronisation for the first time with the beacon received
	if(MAC > s->ID_Beacon){
		s->Counter = DUREE_SLOT*(MAC-s->ID_Beacon-1)+1;				// delay 1.5ms*MAC 
	}else if(MAC < s->ID_Beacon){
		s->Counter = DUREE_CYCLE - DUREE_SLOT*(s->ID_Beacon - MAC +1);			//wait the next cycle		
	}
}

//initialisation for timer A and timer b ; set mode
void Start_Timer(){
	TBCTL=TBSSEL_2 + MC_1;     		  // ACLK = VLO =  12KHz up
	TBCCTL0 = CCIE;                          // TACCR1 interrupt enabled
	TBCCR0 = N_1MS;				 // delay duty scan
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
	s->Counter = DUREE_SCAN;
	Start_Timer();
}

void timer_wait_message(Status * s){				//after sending beacon , wait for sending message
	s->Counter = DUREE_SLOT*(N_SLOT + 1 - MAC);			// delay TIME_SLOT*(N + 1 - MAC) 
	Start_Timer();
}

void timer_host_wait_message(Status * s){				//after sending beacon , wait for sending message
	s->Counter = DUREE_SLOT*(N_SLOT + 1);			// delay TIME_SLOT*(N + 1 - MAC) 
	Start_Timer();
}

void timer_wait_sleep(Status * s){				//after sending message , wait for sleeping
	s->Counter = DUREE_ACTIVE;					// wait the end of message 
	Start_Timer();
}

void timer_wait_beacon(Status * s){
	s->Counter = DUREE_SLOT*MAC + DUREE_SLEEP;			//wait the end of sllep
	Start_Timer();
}

void timer_host_wait_beacon(Status * s){
	s->Counter = DUREE_SLEEP;						//wait the end of sllep
	Start_Timer();
}

