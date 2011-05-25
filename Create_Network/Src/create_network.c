#include "common.h"
#include <mrfi.h> 
#include <string.h> 
#include "interrupt.h"
#include "uart.h"

char Message1[] =  "i  want 1 "; 
char Message2[] =  "je veux 2 "; 
char M1[] =  "i am 1    ";  
char M2[] =  "i am 2    ";  
char M3[] =  "i am 3    "; 
char M4[] =  "i am 4    ";  

Status etat;			//record all the status
uint16_t  N_synchrone ;

void Send_beacon(){				//send the packet of beacon
	mrfiPacket_t beaconToSend;
	beaconToSend.frame[0] = BEACON_SIZE;
	beaconToSend.frame[4] = etat.MAC;
	beaconToSend.frame[8] = BROADCAST;
	//fill in the beacon flag
	beaconToSend.frame[9]  = FBEACON; 
	//fill in the ID_NETWORK and ID_NODE
	beaconToSend.frame[10] = etat.ID_Network;
	if(etat.HOST == IS_CREATER){
		beaconToSend.frame[11] = 0;			//Slot num , host is creater
	}else{
		beaconToSend.frame[11] = etat.MAC;			//Slot num 
	}
	
	//send the beacon
	//MRFI_Transmit(&beaconToSend, MRFI_TX_TYPE_CCA) ;
	MRFI_Transmit(&beaconToSend, MRFI_TX_TYPE_FORCED) ;

}

void Send_message(char Mess[] , uint8_t  Destination){	//send the message
	mrfiPacket_t packetToSend;
	uint8_t i;
	packetToSend.frame[0] = strlen(Mess) + 9 +1;//PAYLOAD_SIZE; add '\n' '\r'
	packetToSend.frame[4] = etat.MAC;
	packetToSend.frame[8] = Destination;
	packetToSend.frame[9] = FDATA; 

	//fill in the payload
	for (i=0;i<strlen(Mess);i++) {
		packetToSend.frame[i+10] = Mess[i];
	}
	packetToSend.frame[i+10] = '\r';
//	packetToSend.frame[i+11] = '\r';
	
	//send the message
	//MRFI_Transmit(&packetToSend, MRFI_TX_TYPE_CCA);
	MRFI_Transmit(&packetToSend, MRFI_TX_TYPE_FORCED);
}

void Sleep(){
	;	
	//__bis_SR_register(GIE+LPM3_bits);	//in sleep and interrupt mode
}

void update_synchron(){
	if(etat.HOST == IS_NOT_CREATER){
		etat.state = WAIT_SCAN;		//change the state
		etat.synchron = 0;
	}
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

	//open sunchrone again every 5s
	//N_synchrone = 1000;			//5s
	//Timer_synchrone();

	etat.state = WAIT_SCAN;			//first time ;initialisation
	etat.ID_Network = NO_NETWORK;			//no network at first
	etat.HOST = IS_NOT_CREATER ;
	etat.synchron = 0;
	etat.MAC = 3;
	etat.ID_Beacon = BROADCAST;
	etat.Counter = 0;
}


int main( void )
{
	Init();
	__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
	
	return 0;
}

void Timer_A0(void);
interrupt(TIMERA0_VECTOR) Timer_A0(void)
{
	N_synchrone--;
	if(N_synchrone == 0){
		update_synchron();
		N_synchrone = 1000;	
//		P1OUT ^= 0x01;   			//rouge led
	}
//	P1OUT ^= 0x02;   			//jaune led
}


void Timer_B0(void);
interrupt(TIMERB0_VECTOR) Timer_B0(void)
{
	etat.Counter--;

	if( etat.HOST == IS_NOT_CREATER && etat.state == WAIT_BEACON && etat.Counter == (DUREE_SLOT*(N_SLOT+1-etat.MAC))){
			P1OUT ^= 0x01;   			//rouge led
			Send_beacon();
	}

	if(etat.Counter == 0){
		Stop_Timer();
		if(etat.ID_Network == NO_NETWORK){
	 		etat.ID_Network = ID_NETWORK_CREATE;
	  		etat.HOST = IS_CREATER;
	 		etat.state = WAIT_BEACON;		//change the state
			etat.synchron = 1;

			timer_wait_message(&etat);
			Send_beacon();
			P1OUT ^= 0x02;   			//jaune led
	 	}else{
			P1OUT ^= 0x01;   			//rouge led

			switch(etat.state){
				case WAIT_BEACON : 
					etat.state = WAIT_MESSAGE;
					timer_wait_sleep(&etat);
					break;
				case WAIT_MESSAGE :
					etat.state = WAIT_SLEEP;
					timer_wait_beacon(&etat);

					//choose the message corresponding his MAC
					switch(etat.MAC){
						case 1 : 
							Send_message(M1, BROADCAST);
							break;
						case 2 : 
							Send_message(M2, BROADCAST);
							break;
						case 3 : 
							Send_message(M3, BROADCAST);
							break;
						case 4 : 
							Send_message(M4, BROADCAST);
							break;
						default: 
							break;
					}				
					//Send_message(Message1 , (uint8_t)1);
					//Send_message(Message2 , (uint8_t)2);
					break;
				case WAIT_SLEEP :
					etat.state = WAIT_BEACON;	
					timer_wait_message(&etat);
					if(etat.HOST == IS_CREATER){
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
	uint8_t i;
	mrfiPacket_t packet;
	char output[10] = "";

	MRFI_Receive(&packet);

	if(packet.frame[9] == FBEACON){
								//if we recieve a packet in state of "wait_beacon" or wait_scan , maybe it is a beancon
		etat.ID_Network = packet.frame[10];		//attention, support only one network
		etat.ID_Beacon  = packet.frame[11];		//the slot_num 	
	
		if(etat.synchron == 0){				//in scan or update
			etat.synchron = 1;
	 		etat.state = WAIT_SLEEP;		//in this state ; it can send beacon
			Stop_Timer();			
			timer_wait_first(&etat);
		}

		//show the ID_NETWORK
		output[0] = etat.ID_Network/10 + '0';
		output[1] = etat.ID_Network%10 + '0';
		//show the MAC of the source
		output[2] = etat.ID_Beacon/10 + '0';
		output[3] = etat.ID_Beacon%10 + '0';
		
		TXString(output, 4) ;//(sizeof output));

	}else if(packet.frame[9] == FDATA){
		if(packet.frame[8] == etat.MAC || packet.frame[8] == BROADCAST){
			for (i=10;i<packet.frame[0];i++) {
				output[i-10]=packet.frame[i];
				if (packet.frame[i]== '\r') {
					output[i-10]='\n';
					output[i-9]='\r';
				}
			}
			TXString(output, (sizeof output));
		}else{			//if the destination is not him, relay
			//MRFI_Transmit(&packet, MRFI_TX_TYPE_CCA);
			MRFI_Transmit(&packet, MRFI_TX_TYPE_FORCED);
		}
	}
}


