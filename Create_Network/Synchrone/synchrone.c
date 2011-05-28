#include "common.h"
#include <mrfi.h> 
#include <string.h> 
#include "interrupt.h"
#include "uart.h" 
#include "send.h" 


Status etat;			//record all the status
//uint16_t  first_wait = 0;

char Message[4][20] = {"hello Node1","hello Node2","hello Node3","hello Node4",};

void Sleep(){
	__bis_SR_register(LPM3_bits + GIE);       // Enter LPM0 w/ interrupt
}


void Init(){
	WDTCTL = WDTPW + WDTHOLD;

	//set clock 8MHZ
	DCOCTL = 0x0;
	BCSCTL1 = CALBC1_8MHZ;                    // ACLK, Set DCO to 8MHz
	DCOCTL  = CALDCO_8MHZ;
	BCSCTL2 |= DIVS_0;                         // SMCLK = DCO / 1
	BCSCTL3 |= LFXT1S_2;			  // set frequency VLO = 12k

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
	}
*/
	if(etat.Counter == 0){
		Stop_Timer();
		if(etat.ID_Network == NO_NETWORK){
	 		etat.ID_Network = etat.MAC;
	  		etat.HOST = IS_CREATER;
			etat.synchrone = 1;
		
	 		etat.state = WAIT_SYNCHRONE;		//change the state
			timer_synchrone(&etat);
			Send_beacon(&etat);

			P1OUT |= 0x02;   			//jaune led
	 	}else{
			switch(etat.state){
				case WAIT_BEACON : 
					etat.state = WAIT_SYNCHRONE;
					timer_synchrone(&etat);
					if(etat.ID_Beacon < etat.MAC){
						P1OUT ^= 0x01;   			//rouge led
						Send_beacon(&etat);			//send beacon
					}else{									
						etat.synchrone = 0;
						Stop_Timer();	
					}		
					break;
				case WAIT_SYNCHRONE : 
					etat.state = WAIT_MESSAGE;
					//time for message
					if(etat.HOST == IS_CREATER){
						Send_message(&etat, Message[etat.MAC-1], 1);
					}
					timer_message(&etat);	
					break;
				case WAIT_MESSAGE :
					etat.state = WAIT_SLEEP;
					timer_sleep(&etat);
					//time for sleep
					Sleep();
					break;
				case WAIT_SLEEP :
					etat.state = WAIT_BEACON;
					__bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)			
					if(etat.HOST == IS_NOT_CREATER ){
						if(etat.ID_Beacon < etat.MAC){
							etat.synchrone = 0;
							Stop_Timer();
						}else{
							timer_send_beacon(&etat);
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
	mrfiPacket_t PacketRecieved;
	mPacket Packet;

	uint8_t ID_Network_tmp, ID_Beacon_tmp ,i;
	char output[MRFI_MAX_FRAME_SIZE-10]="";

	MRFI_Receive(&PacketRecieved);
	RecievemPacket(&PacketRecieved ,&Packet);

	if(Packet.flag == FBEACON){	
		ID_Network_tmp = Packet.payload.beacon.ID_Network;
		ID_Beacon_tmp  = Packet.payload.beacon.ID_Slot;
		if(etat.synchrone == 0){				//in scan or update
			etat.ID_Network = Packet.payload.beacon.ID_Network;		//attention, support only one network
			etat.ID_Beacon  = Packet.payload.beacon.ID_Slot;		//the slot_num 	
			etat.synchrone = 1;
			if(etat.ID_Beacon < etat.MAC){
		 		etat.state = WAIT_BEACON;		//in this state ; it can send beacon
				timer_send_beacon(&etat);				
			}else{
		 		etat.state = WAIT_SYNCHRONE;		//in this state ; it can send beacon
				timer_synchrone(&etat);
			}	
		}

		if(ID_Network_tmp < etat.ID_Network){			//if there is 2 network collision
			if(etat.HOST == IS_CREATER){
				etat.HOST = IS_NOT_CREATER;
				P1OUT ^= 0x02;   			//jaune led
			}	
			etat.ID_Network = ID_Network_tmp;	
			etat.ID_Beacon  = ID_Beacon_tmp;			
			etat.synchrone = 1;
			if(etat.ID_Beacon < etat.MAC){
		 		etat.state = WAIT_BEACON;		//in this state ; it can send beacon
				timer_send_beacon(&etat);				
			}else{
		 		etat.state = WAIT_SYNCHRONE;		//in this state ; it can send beacon
				timer_synchrone(&etat);
			}		
		}

		//show the ID_NETWORK
		output[0] = ID_Network_tmp/10 + '0';
		output[1] = ID_Network_tmp%10 + '0';
		//show the MAC of the source
		output[2] = ID_Beacon_tmp/10 + '0';
		output[3] = ID_Beacon_tmp%10 + '0';

		TXString(output, strlen(output));

	}else if(Packet.flag == FDATA){
		if(Packet.dst[3] == etat.MAC || Packet.dst[3] == BROADCAST){
			for (i=0;i<Packet.length-9;i++) {
				output[i] = Packet.payload.data[i];
			}
			TXString(output, strlen(output));
		}else{			//if the destination is not him, relay
			//MRFI_Transmit(&packet, MRFI_TX_TYPE_CCA);
			MRFI_Transmit(&PacketRecieved, MRFI_TX_TYPE_FORCED);
		}
	}
}


