#include "common.h"
#include <mrfi.h> 
#include <string.h> 
#include "interrupt.h"
#include "uart.h"

char Message[] = "bonjour " ;
char wait_b[] = "w_b " ;
char wait_m[] = "w_m " ;
char wait_s[] = "w_s " ;
char beacon_OK[] = "b_ok " ;
char message_OK[] = "m_ok " ;
char sleep_OK[] = "s_ok " ;

volatile uint8_t HOST;

void Send_beacon(){				//send the packet of beacon
	mrfiPacket_t beaconToSend;
	beaconToSend.frame[0] = BEACON_SIZE;
	beaconToSend.frame[4] = MAC;
	beaconToSend.frame[8] = BROADCAST;
	//fill in the beacon flag
	beaconToSend.frame[9]  = FBEACON; 
	//fill in the ID_NETWORK and ID_NODE
	beaconToSend.frame[10] = ID_Network;
	beaconToSend.frame[11] = MAC;			//Slot num 
	
	//send the beacon
	MRFI_Transmit(&beaconToSend, MRFI_TX_TYPE_FORCED);
	TXString(beacon_OK, (sizeof beacon_OK));
}

void Send_message(char Mess[] , uint8_t  Destination){	//send the message
	mrfiPacket_t packetToSend;
	uint8_t i;
	packetToSend.frame[0] = strlen(Mess) + 9 +1;//PAYLOAD_SIZE; add '\n' '\r'
	packetToSend.frame[4] = MAC;
	packetToSend.frame[8] = Destination;
	packetToSend.frame[9] = FDATA; 

	//fill in the payload
	for (i=0;i<strlen(Mess);i++) {
		packetToSend.frame[i+10] = Mess[i];
	}
	packetToSend.frame[i+10] = '\r';
//	packetToSend.frame[i+11] = '\r';
	
	//send the message
	MRFI_Transmit(&packetToSend, MRFI_TX_TYPE_FORCED);
	TXString(message_OK, (sizeof message_OK));
}

void Sleep(){
	;	
	TXString(sleep_OK, (sizeof sleep_OK));
	//__bis_SR_register(GIE+LPM3_bits);	//in sleep and interrupt mode
}

void Init(){
	WDTCTL = WDTPW + WDTHOLD;
	P1DIR |= 0x03;                            // P1.0 output
	P1OUT |= 0x02;
	BCSCTL3 |= LFXT1S_2;

	Button_Init();
	Timer_Init();
	Uart_Init();

	state = WAIT_BEACON;				//first time ;initialisation
	ID_Network = NO_NETWORK;			//no network at first
	HOST = IS_NOT_CREATER ;
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
	if(ID_Network == NO_NETWORK){
 		ID_Network = ID_NETWORK_CREATE;
  		HOST = IS_CREATER;
 		state = WAIT_MESSAGE;		//change the state
		timer_host_wait_message();
		Send_beacon();
		P1OUT ^= 0x02;   			//jaune led
 	}else{
		P1OUT ^= 0x01;   			//rouge led
		switch(state){
			case WAIT_BEACON : 
				state = WAIT_MESSAGE;
				if(HOST == IS_CREATER){
					timer_host_wait_message();
				}else{
					timer_wait_message();
				}
				TXString(wait_b, (sizeof wait_b));
				Send_beacon();
				break;
			case WAIT_MESSAGE :
				state = WAIT_SLEEP;
				timer_wait_sleep();
				TXString(wait_m, (sizeof wait_m));
				Send_message(Message,BROADCAST);
				break;
			case WAIT_SLEEP :
				state = WAIT_BEACON;	
				if(HOST == IS_CREATER ){
					timer_host_wait_beacon();
				}else{
					timer_wait_beacon();
				}
				TXString(wait_s, (sizeof wait_s));
				Sleep();
				break;
			default:
				break;		
		}
	}
}

void Buttopn(void);
interrupt(PORT1_VECTOR) Buttopn(void)
{
	P1IFG &= ~0x04;
	P1OUT ^=  0x03;
	Scan_Init();		//open timer B for scan

	//after press the button , we can send and recieve message 
	BSP_Init();
	MRFI_Init(); 
	MRFI_WakeUp();
	MRFI_RxOn(); 
}


void MRFI_RxCompleteISR()
{
	uint8_t i;
	mrfiPacket_t packet;
	char output[] = {"                   "};

	MRFI_Receive(&packet);

	if(state == WAIT_BEACON && packet.frame[9] == FBEACON){			//if we recieve a packet in state of "wait_beacon" , maybe it is a beancon
		ID_Network = packet.frame[10];	
		ID_Beacon  = packet.frame[11];
		wait_beacon_first(ID_Beacon);

		//show the ID_NETWORK
		output[0] = ID_Network/100 + '0';
		output[1] = ID_Network%100/10 + '0';
		output[2] = ID_Network%10 + '0';
		output[3] = ' ';

		//show the MAC of the source
		output[4] = ID_Beacon/100 + '0';
		output[5] = ID_Beacon%100/10 + '0';
		output[6] = ID_Beacon%10 + '0'; 
		output[7] = '\n';
		output[8] = '\r';

	}else if(state == WAIT_SLEEP && packet.frame[9] == FDATA){
		for (i=10;i<packet.frame[0];i++) {
			output[i-10]=packet.frame[i];
			if (packet.frame[i]== 0) {
				output[i-10]='\n';
				output[i-9]='\r';
			}
		}
	}

	TXString(output, (sizeof output));
}


