#include "common.h"
#include <mrfi.h> 
#include <string.h> 
#include "interrupt.h"
#include "uart.h" 
#include "send.h" 


Status etat;			//record all the status
//uint16_t  first_wait = 0;


char Message[4][5] = {"iam1","iam2","iam3","iam4"};


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
	etat.MAC = 1;
	etat.HOST = IS_NOT_CREATER ;
	etat.synchrone = 0;
	etat.ID_Beacon = BROADCAST;
	etat.Beacon_sent = 0;
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
	/*
	if(etat.synchrone == 1&&etat.HOST == IS_NOT_CREATER && etat.state == WAIT_MESSAGE && etat.Counter == (uint16_t) ( DUREE_ACTIVE - etat.MAC) ){
		Send_message(&etat, Message[etat.MAC-1], BROADCAST);
	}*/

	if(etat.Counter == 0){
		Stop_Timer();
		if(etat.ID_Network == NO_NETWORK){
	 		etat.ID_Network = etat.MAC;
	  		etat.HOST = IS_CREATER;
			etat.synchrone = 1;
		
	 		etat.state = WAIT_SYNCHRONE;		//change the state
			timer_synchrone(&etat);
			Send_beacon(&etat);
			etat.Beacon_sent = 1;	

			P1OUT |= 0x02;   			//jaune led
	 	}else{
			switch(etat.state){
				case WAIT_BEACON : 
					etat.state = WAIT_SYNCHRONE;
					timer_synchrone(&etat);
					if(etat.ID_Beacon < etat.MAC){
						P1OUT ^= 0x01;   			//rouge led
						Send_beacon(&etat);			//send beacon
						etat.Beacon_sent = 1;
					}else{									
						etat.synchrone = 0;
						//etat.Beacon_sent = 1; 
						Stop_Timer();	
					}		
					break;
				case WAIT_SYNCHRONE : 
					etat.state = WAIT_MESSAGE;
					/*if(etat.HOST == IS_CREATER){
						Send_message(&etat, Message[etat.MAC-1], BROADCAST);
					}*/
					timer_message(&etat);	
					break;
				case WAIT_MESSAGE :
					etat.state = WAIT_SLEEP;
					timer_sleep(&etat);
					break;
				case WAIT_SLEEP :
					etat.state = WAIT_BEACON;
					if(etat.HOST == IS_NOT_CREATER ){
						if(etat.ID_Beacon < etat.MAC){
							etat.synchrone = 0;
							//etat.Beacon_sent = 1; 
							Stop_Timer();
						}else{
							if(etat.Beacon_sent == 0){ 	
								timer_send_beacon(&etat);
							}
						}
					}else{						//the host
				 		etat.state = WAIT_SYNCHRONE;		//change the state
						timer_synchrone(&etat);
						P1OUT ^= 0x01;   			//rouge led
						Send_beacon(&etat);
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
	MRFI_SetLogicalChannel(1);
	MRFI_SetRFPwr(0);

	Uart_Init();

	MRFI_WakeUp();
	MRFI_RxOn(); 
}

void MRFI_RxCompleteISR()
{
	mrfiPacket_t packet;
	uint8_t i;
	char output[10] = "";

	MRFI_Receive(&packet);

	if(packet.frame[9] == FBEACON){	
								//if we recieve a packet in state of "wait_beacon" or wait_scan , maybe it is a beancon
		if(etat.synchrone == 0){				//in scan or update
			etat.ID_Network = packet.frame[10];		//attention, support only one network
			etat.ID_Beacon  = packet.frame[11];		//the slot_num 	

			if(etat.ID_Network == etat.MAC){		//if the HOST reset
		  		etat.HOST = IS_CREATER;
		 		etat.state = WAIT_SYNCHRONE;		//change the state
				timer_synchrone(&etat);
				P1OUT |= 0x02;   			//jaune led
			}else{
				etat.synchrone = 1;
				etat.Beacon_sent = 0 ;
				if(etat.ID_Beacon < etat.MAC){
			 		etat.state = WAIT_BEACON;		//in this state ; it can send beacon
					timer_send_beacon(&etat);				
				}else{
			 		etat.state = WAIT_SYNCHRONE;		//in this state ; it can send beacon
					timer_synchrone(&etat);
				}
			}
		}

		//show the ID_NETWORK
		output[0] = packet.frame[10]/10 + '0';
		output[1] = packet.frame[10]%10 + '0';
		//show the MAC of the source
		output[2] = packet.frame[11]/10 + '0';
		output[3] = packet.frame[11]%10 + '0';

		TXString(output, 4); //(sizeof output));

	}else if(packet.frame[9] == FDATA){
		if(packet.frame[8] == etat.MAC || packet.frame[8] == BROADCAST){
			for (i=10;i<packet.frame[0];i++) {
				output[i-10]=packet.frame[i];
				if (packet.frame[i]== '\r') {
					output[i-10]='\n';
					output[i-9]='\r';
				}
			}
			TXString(output, 4);//(sizeof output));
		}else{			//if the destination is not him, relay
			//MRFI_Transmit(&packet, MRFI_TX_TYPE_CCA);
			MRFI_Transmit(&packet, MRFI_TX_TYPE_FORCED);
		}
	}
}

