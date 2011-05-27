#include "common.h"
#include "uart.h"
#include "interrupt.h"
#include "string.h"
#include <mrfi.h> 
#include "send.h"


void Send_beacon(Status * s){				//send the packet of beacon
	char output[4] = "";

	mrfiPacket_t beaconToSend;
	beaconToSend.frame[0] = BEACON_SIZE + 4 ;
	beaconToSend.frame[4] = s->MAC;
	beaconToSend.frame[8] = BROADCAST;
	//fill in the beacon flag
	beaconToSend.frame[9]  = FBEACON; 
	//fill in the ID_NETWORK and ID_NODE
	beaconToSend.frame[10] = s->ID_Network;
	if(s->HOST == IS_CREATER){
		beaconToSend.frame[11] = 0;			//Slot num , host is creater
	}else{
		beaconToSend.frame[11] = s->MAC;			//Slot num 
	}

	//send the beacon
	MRFI_Transmit(&beaconToSend, MRFI_TX_TYPE_FORCED);

	output[0] = s->ID_Network/10 + '0';
	output[1] = s->ID_Network%10 + '0';
	if(s->HOST == IS_CREATER){
		output[2] = '0';
		output[3] = '0';
	}else{
		output[2] = s->MAC/10 + '0';
		output[3] = s->MAC%10 + '0';
	}
//	TXString(output, 4); //(sizeof output));
}


void Send_message(Status * s, char * Mess , uint8_t  Destination){	//send the message

	mrfiPacket_t packetToSend;
	uint8_t i;
	packetToSend.frame[0] = 4 + 9 +1;//PAYLOAD_SIZE; add '\n' '\r'
	packetToSend.frame[4] = s->MAC;
	packetToSend.frame[8] = Destination;
	packetToSend.frame[9] = FDATA; 

	//fill in the payload
	for (i=0;i<strlen(Mess);i++) {
		packetToSend.frame[i+10] = Mess[i];
	}
	packetToSend.frame[i+10] = '\r';
	//send the message
	//MRFI_Transmit(&packetToSend, MRFI_TX_TYPE_CCA);
	MRFI_Transmit(&packetToSend, MRFI_TX_TYPE_FORCED);

//	TXString(Mess, 4);
}

