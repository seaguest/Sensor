#include "common.h"
#include <mrfi.h> 
#include <string.h> 
#include "interrupt.h"
#include "uart.h" 

Status etat;			//record all the status
//uint16_t  first_wait = 0;

void Delay_before_slot(){		//BEACON_SIZE + 1 + 2 = 14B = 1.552ms = 12416
	volatile uint16_t y = 12000;
	while(y--);	
}

void Send_beacon(){				//send the packet of beacon
	mrfiPacket_t beaconToSend;
	beaconToSend.frame[0] = BEACON_SIZE;
	beaconToSend.frame[4] = etat.MAC;
	beaconToSend.frame[8] = BROADCAST;
	//fill in the beacon flag
	beaconToSend.frame[9]  = FBEACON; 
	//fill in the ID_NETWORK and ID_NODE
	beaconToSend.frame[10] = etat.ID_Network;
	if(etat.HOST == etat.MAC){
		beaconToSend.frame[11] = 0;			//Slot num , host is creater
	}else{
		beaconToSend.frame[11] = etat.MAC;			//Slot num 
	}
	//send the beacon
	MRFI_Transmit(&beaconToSend, MRFI_TX_TYPE_FORCED);
//	TXString(beacon_OK, (sizeof beacon_OK));
}

void Sleep(){
	;	
}


void Init(){
	WDTCTL = WDTPW + WDTHOLD;

	//set clock 8MHZ
	DCOCTL = 0x0;
	BCSCTL1 = CALBC1_8MHZ;                    // ACLK, Set DCO to 8MHz
	DCOCTL  = CALDCO_8MHZ;
	BCSCTL2 |= DIVS_0;                         // SMCLK = DCO / 1

	//set LED
	P1DIR |= 0x03;                            // P1.0 output
	P1OUT |= 0x02;

	Button_Init();

	etat.state = WAIT_SCAN;			//first time ;initialisation
	etat.ID_Network = NO_NETWORK;			//no network at first
	etat.MAC = 2;
	etat.HOST = BROADCAST ;
	etat.synchron = 0;
	etat.ID_Beacon = BROADCAST;
	etat.Counter = 0;
}


int main( void )
{
	Init();
	__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
	
	return 0;
}


void Timer_B0(void);
interrupt(TIMERB0_VECTOR) Timer_B0(void)
{
	etat.Counter--;
	if( etat.HOST != etat.MAC && etat.state == WAIT_BEACON && etat.Counter == (uint16_t) (DUREE_SLOT*(N_SLOT + 1 - etat.MAC))){
			P1OUT ^= 0x01;   			//rouge led
			Send_beacon();
	}

	if(etat.Counter == 0){
		Stop_Timer();
		if(etat.ID_Network == NO_NETWORK){
	 		etat.ID_Network = ID_NETWORK_CREATE;
	  		etat.HOST = etat.MAC;
	 		etat.state = WAIT_BEACON;		//change the state
			etat.synchron = 1;

			timer_wait_message(&etat);
			Send_beacon();
			P1OUT ^= 0x02;   			//jaune led
	 	}else{
			//P1OUT ^= 0x01;   			//rouge led
			switch(etat.state){
				case WAIT_BEACON : 
					etat.state = WAIT_MESSAGE;
					timer_wait_sleep(&etat);
					break;
				case WAIT_MESSAGE :
					etat.state = WAIT_SLEEP;
					timer_wait_beacon(&etat);
					break;
				case WAIT_SLEEP :
					etat.state = WAIT_BEACON;	
					timer_wait_message(&etat);
					if(etat.HOST == etat.MAC){
						P1OUT ^= 0x01;   			//rouge led
						Send_beacon();
					}
					break;
				default:
					break;		
			}
		}

	}

}

void Buttopn(void);
interrupt(PORT1_VECTOR) Buttopn(void)
{
	P1IFG &= ~0x04;
	P1OUT ^=  0x03;
	Scan_Init(&etat);		//open timer B for scan

	//after press the button , we can send and recieve message 
	BSP_Init();
	MRFI_Init();

	Uart_Init();

	MRFI_WakeUp();
	MRFI_RxOn(); 
}

void MRFI_RxCompleteISR()
{
	mrfiPacket_t packet;
	char output[] = {"    "};
//	P1OUT ^= 0x02;
	MRFI_Receive(&packet);

	if((etat.state == WAIT_SCAN ||etat.state == WAIT_BEACON) && packet.frame[9] == FBEACON){	
								//if we recieve a packet in state of "wait_beacon" or wait_scan , maybe it is a beancon
		etat.ID_Network = packet.frame[10];		//attention, support only one network
		etat.ID_Beacon  = packet.frame[11];		//the slot_num 	
	
		if(etat.synchron == 0){				//in scan or update
			etat.synchron = 1;
	 		etat.state = WAIT_SLEEP;		//in this state ; it can send beacon
			
			Stop_Timer();			
			Delay_before_slot();			//wait for 1.552ms

			timer_wait_first(&etat);

		}

		//show the ID_NETWORK
		output[0] = etat.ID_Network/10 + '0';
		output[1] = etat.ID_Network%10 + '0';
		//show the MAC of the source
		output[2] = etat.ID_Beacon/10 + '0';
		output[3] = etat.ID_Beacon%10 + '0';

		TXString(output, (sizeof output));

	}
}


