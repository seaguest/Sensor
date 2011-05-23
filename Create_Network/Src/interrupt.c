#include "common.h"
#include "interrupt.h"
#include <mrfi.h> 


void wait_beacon_first(uint8_t ID_Beacon){			//synchronisation for the first time with the beacon received
	TBCCTL0 = CCIE;

	if(MAC > ID_Beacon){
		TBCCR0 = DUREE_SLOT*(MAC-ID_Beacon-1)+1;				// delay 1.5ms*MAC 
	}else if(MAC < ID_Beacon){
		TBCCR0 = DUREE_CYCLE - DUREE_SLOT*(ID_Beacon - MAC +1);			//wait the next cycle		
	}
}

//initialisation for timer A and timer b ; set mode
void Timer_Init(){
	TBCTL=TBSSEL_1 + MC_1;     		  // ACLK = VLO =  12KHz up
}

//initialisation for button
void Button_Init(){
	P1DIR |=  0x03;
	P1REN |=  0x04;
	P1OUT |=  0x03;
	P1IE  |=  0x04;

	BSP_Init();
//	MRFI_Init(); 
//	MRFI_WakeUp();
//	MRFI_RxOn(); 
}

void Scan_Init(void)				//open the timer of scan ; after the time over ; if not existe a network ;then create one
{
	TBCCTL0 = CCIE;                         // TACCR1 interrupt enabled
	TBCCR0 = DUREE_SCAN;			 // delay duty scan
}

void timer_wait_message(){				//after sending beacon , wait for sending message
	TBCCTL0 = CCIE;
	TBCCR0 = DUREE_SLOT*(N_SLOT + 1 - MAC);			// delay TIME_SLOT*(N + 1 - MAC) 
}

void timer_host_wait_message(){				//after sending beacon , wait for sending message
	TBCCTL0 = CCIE;
	TBCCR0 = DUREE_SLOT*(N_SLOT + 1);			// delay TIME_SLOT*(N + 1 - MAC) 
}

void timer_wait_sleep(){				//after sending message , wait for sleeping
	TBCCTL0=CCIE;
	TBCCR0 = DUREE_ACTIVE;					// wait the end of message 
}

void timer_wait_beacon(){
	TBCCTL0=CCIE;
	TBCCR0 = DUREE_SLOT*(MAC-1) + DUREE_SLEEP;			//wait the end of sllep
}

void timer_host_wait_beacon(){
	TBCCTL0=CCIE;
	TBCCR0 = DUREE_SLEEP;						//wait the end of sllep
}

