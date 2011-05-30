#include "common.h"
#include "uart.h"
#include "interrupt.h"
#include "string.h"
#include <mrfi.h> 
#include "cycle.h"
#include "fifo.h"


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
		dst->frame[12] = (uint8_t) (src->payload.beacon.Voisin/(255*255*255));
		dst->frame[13] = (uint8_t) ((src->payload.beacon.Voisin%(255*255*255))/(255*255));
		dst->frame[14] = (uint8_t) ((src->payload.beacon.Voisin%(255*255))/255);
		dst->frame[15] = (uint8_t) (src->payload.beacon.Voisin%255);
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
		dst->payload.beacon.Voisin = src->frame[12]*255*255*255 + src->frame[13]*255*255 +src->frame[14]*255 + src->frame[15];
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
	
	Packet.payload.beacon.Voisin = s->Voisin;

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
	TXString(output, 4);
}

void Recieve_message(Status * s, QList *Q){	//Recieve the message
	char output[MRFI_MAX_FRAME_SIZE-10]="";
	uint8_t i;
	char c;

	i = 0;
	while(Search(Q, '\r')){				 
		c = DeQueue(Q);
		if( c == '\r' ){
			output[i] = '\n';		
			output[i+1] = '\r';		
			i = 0;
			TXString(output, strlen(output));
		}else{
			output[i] = c;	
			i++;	
		}
	}	
}

void Send_message(Status * s, QList *Q, uint8_t  Destination){	//send the message

	mrfiPacket_t PacketToSend;
	mPacket Packet;

	uint8_t i ;

	if(!IsEmpty(Q)){
		Packet.src[3] = s->MAC;
		Packet.dst[3] = Destination;
		Packet.flag = FDATA; 
	
		if(Length(Q)>=PAYLOAD_MAX_SIZE){
			Packet.length = PAYLOAD_MAX_SIZE + 10; 
			for(i = 0;i<PAYLOAD_MAX_SIZE;i++){
				Packet.payload.data[i] = DeQueue(Q);
			}
		}else{
			i = 0;
			Packet.length = Length(Q) + 10;
			while(!IsEmpty(Q)){
				Packet.payload.data[i] = DeQueue(Q);
				i++;
			}
		}

		SendmPacket(&Packet, &PacketToSend);

		//send the message
		//MRFI_Transmit(&packetToSend, MRFI_TX_TYPE_CCA);
		MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_FORCED);
	}
 
}

void Sleep(){
	__bis_SR_register(LPM3_bits + GIE);       // Enter LPM0 w/ interrupt
}
