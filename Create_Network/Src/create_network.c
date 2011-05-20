#include "common.h"
#include <mrfi.h> 
#include <string.h> 
#include "interrupt.h"
#include "uart.h"

#define MAC KH_2	//choose the MAC

char Message[] = "bonjour " ;

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
}

void Send_message(char Mess[] , uint8_t  Destination){	//send the message
	mrfiPacket_t packetToSend;
	uint8_t i;
	packetToSend.frame[0] = PAYLOAD_SIZE;
	packetToSend.frame[4] = MAC;
	packetToSend.frame[8] = Destination;
	packetToSend.frame[9] = FDATA; 

	//fill in the payload
	for (i=0;i<strlen(Mess);i++) {
		packetToSend.frame[i+10] = Mess[i];
	}
	
	//send the message
	MRFI_Transmit(&packetToSend, MRFI_TX_TYPE_FORCED);
}

void Sleep(){
	__bis_SR_register(GIE+LPM3_bits);	//in sleep and interrupt mode
}

int main( void )
{
	WDTCTL = WDTPW + WDTHOLD;
	P1DIR |=  0x03;
	BCSCTL3 |= LFXT1S_2;
	TACCTL0 = CCIE;
	TACCR0 = 5000;
	TACTL = MC_1+TASSEL_1;
	
	Button_Init();
	Timer_Init();
	Uart_Init();

	state = WAIT_BEACON;		//first time ;initialisation
	ID_Network = NO_NETWORK;			//no network at first

	BSP_Init();
	MRFI_Init(); 
	MRFI_WakeUp();
	MRFI_RxOn(); 

	__bis_SR_register(GIE+LPM3_bits);
	return 0;
}


void Timer_A(void);
interrupt(TIMERA0_VECTOR) Timer_A(void)
{
	switch(state){
		case WAIT_BEACON : 
			state = WAIT_MESSAGE;
			wait_message();
			Send_beacon();
			break;
		case WAIT_MESSAGE :
			state = WAIT_SLEEP;
			wait_sleep();
			Send_message(Message,KH_1);
			break;
		case WAIT_SLEEP :
			state = WAIT_BEACON;	
			wait_beacon();
			Sleep();
			break;
		default:
			break;		
	}
	P1OUT ^= 0x01;
}

void Buttopn(void);
interrupt(PORT1_VECTOR) Buttopn(void)
{
	P1IFG &= ~0x04;
	P1OUT ^=  0x03;
	Scan_Init();		//open timer B for scan
}

// Timer B0 interrupt service routine
void Scan_Network (void);
interrupt(TIMERB0_VECTOR) Scan_Network(void)
{
	if(ID_Network == NO_NETWORK){
		ID_Network = ID_NETWORK_CREATE;
		Send_beacon();
	}
	P1OUT ^= 0x02;   
	TBCCTL0 = ~CCIE;                         // TACCR1 interrupt enabled
}


void MRFI_RxCompleteISR()
{
	uint8_t i;
	mrfiPacket_t packet;
	char output[] = {"                   "};

	MRFI_Receive(&packet);

	if(state == WAIT_BEACON){			//if we recieve a packet in state of "wait_beacon" , maybe it is a beancon
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

	}else if(state == WAIT_SLEEP){
		for (i=10;i<PAYLOAD_SIZE;i++) {
			output[i-10]=packet.frame[i];
			if (packet.frame[i]=='\r') {
				output[i-10]='\n';
				output[i-9]='\r';
			}
		}
	}

	TXString(output, (sizeof output));
}


