#include "common.h"
#include "uart.h"
#include "interrupt.h"
#include "string.h"
#include <mrfi.h> 
#include "cycle.h"
#include "fifo.h"
#include "route.h"
#include <mrfi.h> 


/*
*	this file records the activites during one duty cycle
*	contains the functions of sending message , sending beacon ,sending rip
*/

/*
*	transform the struct to the mrfiPacket for sending	 
*/
void SendmPacket(mPacket *src ,mrfiPacket_t *dst){
	uint8_t i;
	uint32_t voisin; 

	dst->frame[0] = src->length;
	dst->frame[4] = src->src;
	dst->frame[8] = src->dst;
	dst->frame[9] = src->flag;
	if(src->flag == FDATA){
		dst->frame[10] = src->payload.data.Next_hop;
		for(i = 0;i < src->length-11;i++){
			dst->frame[i+11] = src->payload.data.data[i];
		}
	}else if(src->flag == FRIP){
		for(i=0;i<32;i++){					 
			if(src->payload.route[i].Dst!=0){ 
				dst->frame[10+3*i] = src->payload.route[i].Dst;
				dst->frame[11+3*i] = src->payload.route[i].Next_hop;
				dst->frame[12+3*i] = src->payload.route[i].Metric;
			}
		}
	}else if(src->flag == FBEACON){
		dst->frame[10] = src->payload.beacon.ID_Network;
		dst->frame[11] = src->payload.beacon.ID_Slot;
		voisin = src->payload.beacon.Voisin;
		dst->frame[12] = (uint8_t)(voisin/16777216);
		dst->frame[13] = (uint8_t)(voisin%16777216/65536);
		dst->frame[14] = (uint8_t)(voisin>>8);
		dst->frame[15] = (uint8_t)(voisin);
	}	
}

/*
*	transform the mrfiPacket to the struct for recieving
*/
void RecievemPacket(mrfiPacket_t *src ,mPacket *dst){
	uint8_t i;
	dst->length = src->frame[0];
	dst->src = src->frame[4];
	dst->dst = src->frame[8];
	dst->flag = src->frame[9];
	if(dst->flag == FDATA){
		dst->payload.data.Next_hop = src->frame[10];
		for(i = 0;i < src->frame[0]-11;i++){
			dst->payload.data.data[i] = src->frame[i+11];
		}
	}else if(dst->flag == FRIP){
		for(i=0;i<(src->frame[0]-10)/3;i++){		
			dst->payload.route[i].Dst = src->frame[10+3*i];
			dst->payload.route[i].Next_hop = src->frame[11+3*i];
			dst->payload.route[i].Metric = src->frame[12+3*i];;
		}
	}else if(dst->flag == FBEACON){
		dst->payload.beacon.ID_Network = src->frame[10];
		dst->payload.beacon.ID_Slot = src->frame[11];
		dst->payload.beacon.Voisin = ((uint32_t )src->frame[12]*16777216) + ((uint32_t )src->frame[13]*65536) + ((uint32_t )src->frame[14]*256) + (uint32_t )src->frame[15] ; 
	}	
}

/*
*	send beacon 
*/
void Send_beacon(volatile Status * s){				//send the packet of beacon
	mrfiPacket_t PacketToSend;
	mPacket Packet;

	Packet.length = BEACON_SIZE;
	Packet.src = s->MAC;
	Packet.dst = BROADCAST;
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
//	MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_CCA);
	MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_FORCED);	 
}

/*
*	send rip 
*/
void Send_rip(volatile Status * s){			
	uint8_t i, cnt=0;
	mrfiPacket_t PacketToSend;
	mPacket Packet;

	Packet.src = s->MAC;
	Packet.dst = BROADCAST;
	//fill in the RIP flag
	Packet.flag  = FRIP; 

	for(i=0;i<32;i++){					 
		if(s->Route_table[i].Dst!=0){ 
			Packet.payload.route[cnt].Dst = s->Route_table[i].Dst;
			Packet.payload.route[cnt].Next_hop = s->Route_table[i].Next_hop;
			Packet.payload.route[cnt].Metric = s->Route_table[i].Metric;
			cnt++;
		}
	}

	Packet.length =  10 + 3*cnt;
	SendmPacket(&Packet, &PacketToSend);

	//if table is not empty then send the rip
	if(cnt!=0){
		MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_CCA);
		//MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_FORCED);
	}
}

/*
*	read the message in the FIFO_Recieve and find if there is '\r' , if yes then print 
*/
void Recieve_message(volatile Status * s, volatile QList *p){	//Recieve the message
	char output[30]="";
	uint8_t i ,k;
	char c;

	i = 0;
	if(!IsEmpty(p)){
		while(Search(p, '\r')){		//we use the key 'Enter' as a separator for the sentences
			i = 0;
			for(k=0;k<30;k++){
				output[k]=0;
			}
			output[i++] = s->Dst/10 + '0';		
			output[i++] = s->Dst%10 + '0';		
			output[i++] = ':';	
			while((c = DeQueue(p)) != '\r'){
				output[i] = c;	
				i++;	
			}	
			output[i] = '\n';		
			output[i+1] = '\r';	
			print(output);
		}
	}
}

/*
*	write the message to the FIFO_Send with the adr 
*/
void Send_message(volatile Status * s, volatile QList *Q, uint8_t  dst){	//send the message

	mrfiPacket_t PacketToSend;
	mPacket Packet;

	uint8_t i ;
	uint8_t c ;

	if(!IsEmpty(Q)){
		//fill in the header
		Packet.src = s->MAC;
		Packet.dst = dst;
		Packet.flag = FDATA; 
		//find the next hop
		if(Is_voisin(s,dst)){
			Packet.payload.data.Next_hop = dst;
		}else{
			Packet.payload.data.Next_hop = Find_next_hop(s , dst);
		}

		//if there is enough char
		if(Length(Q) >= MRFI_MAX_FRAME_SIZE - 11){
			Packet.length = MRFI_MAX_FRAME_SIZE;
			for(i = 0; i<MRFI_MAX_FRAME_SIZE - 11; i++){
				Packet.payload.data.data[i] = DeQueue(Q) ;
			}
			SendmPacket(&Packet, &PacketToSend);
			MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_CCA);
			//MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_FORCED);
		}else{		//if there is sentence complete in FIFO
			while(Search(Q, '\r')){	
				i = 0;
				while(!IsEmpty(Q)){
					c = DeQueue(Q);
					Packet.payload.data.data[i] = c ;
					i++;
					if(c == '\r'){
						break;
					}
				}
				Packet.length = i + 11;
				SendmPacket(&Packet, &PacketToSend);

				//send the message
				MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_CCA);
				//MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_FORCED);
			}
		}
	}
}

/*
*	sleeo , jump the mode LPM3
*/
void Sleep(void ){
	;//__bis_SR_register(LPM3_bits + GIE);       // Enter LPM0 w/ interrupt
}
