#include "common.h"
#include "uart.h"
#include "interrupt.h"
#include "string.h"
#include <mrfi.h> 
#include "send.h"

void SendmPacket(mPacket *src ,mrfiPacket_t *dst){
	uint8_t i;
	dst->frame[0] = src->length;
	for(i = 0;i <4;i++){
		dst->frame[i+1] = src->src[i];
	}
	for(i = 0;i <4;i++){
		dst->frame[i+5] = src->dst[i];
	}
	dst->frame[9] = src->flag;
	if(src->flag == FDATA){
		for(i = 0;i < src->length-9;i++){
			dst->frame[i+10] = src->payload.data[i];
		}
	}else{
		dst->frame[10] = src->payload.beacon.ID_Network;
		dst->frame[11] = src->payload.beacon.ID_Slot;
		dst->frame[12] = src->payload.beacon.slot_total;
	}	
}

void RecievemPacket(mrfiPacket_t *src ,mPacket *dst){
	uint8_t i;
	dst->length = src->frame[0];
	for(i = 0;i <4;i++){
		dst->src[i] = src->frame[i+1];
	}
	for(i = 0;i <4;i++){
		dst->dst[i] = src->frame[i+5];
	}
	dst->flag = src->frame[9];
	if(src->frame[9] == FDATA){
		for(i = 0;i < src->frame[0]-9;i++){
			dst->payload.data[i] = src->frame[i+10];
		}
	}else{
		dst->payload.beacon.ID_Network = src->frame[10];
		dst->payload.beacon.ID_Slot = src->frame[11];
		dst->payload.beacon.slot_total = src->frame[12];
	}	
}


void Send_beacon(Status * s){				//send the packet of beacon
	char output[4] = "";

	mrfiPacket_t PacketToSend;
	mPacket Packet;

	Packet.length = BEACON_SIZE;
	Packet.src[3] = s->MAC;
	Packet.dst[3] = BROADCAST;
	//fill in the beacon flag
	Packet.flag  = FBEACON; 
	//fill in the ID_NETWORK and ID_NODE
	
	Packet.payload.beacon.ID_Network = s->ID_Network;

	if(s->HOST == IS_CREATER){
		Packet.payload.beacon.ID_Slot = 0;
	}else{
		Packet.payload.beacon.ID_Slot = s->MAC;
	}

	SendmPacket(&Packet, &PacketToSend);

	//send the beacon
	MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_FORCED);

	output[0] = s->ID_Network/10 + '0';
	output[1] = s->ID_Network%10 + '0';
	if(s->HOST == IS_CREATER){
		output[2] = '0';
		output[3] = '0';
	}else{
		output[2] = s->MAC/10 + '0';
		output[3] = s->MAC%10 + '0';
	}
	TXString(output, strlen(output));
}


void Send_message(Status * s, char * Mess , uint8_t  Destination){	//send the message

	mrfiPacket_t PacketToSend;
	mPacket Packet;

	uint8_t i;
	Packet.length = strlen(Mess) + 10 + 1;//PAYLOAD_SIZE; add '\n' '\r'
	Packet.src[3] = s->MAC;
	Packet.dst[3] = Destination;
	Packet.flag = FDATA; 

	//fill in the payload
	for (i=0;i<strlen(Mess);i++) {
		Packet.payload.data[i] = Mess[i];
	}
	Packet.payload.data[i] = 0;		//symble of end

	SendmPacket(&Packet, &PacketToSend);

	//send the message
	//MRFI_Transmit(&packetToSend, MRFI_TX_TYPE_CCA);
	MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_FORCED);
}

