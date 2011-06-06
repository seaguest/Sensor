#include "common.h"
#include "uart.h"
#include "interrupt.h"
#include "string.h"
#include <mrfi.h> 
#include "cycle.h"
#include "fifo.h"
#include "route.h"

/*
*	this file records the activites during one duty cycle
*	contains the functions of sending message , sending beacon ,sending rip
*/


/*
*	transform the struct to the mrfiPacket for sending	 
*/
void SendmPacket(mPacket *src ,mrfiPacket_t *dst){
	//char output[10] = "";
	//char ss[13] = "";
	uint8_t i;
	uint32_t tmp;
	//uint32_t s1 = 255;
	//uint32_t s2 = 255*255;
	//uint32_t s3 = 255*255*255;


	dst->frame[0] = src->length;
	for(i = 0;i <4;i++){
		dst->frame[i+1] = src->src[i];
	}
	for(i = 0;i <4;i++){
		dst->frame[i+5] = src->dst[i];
	}
	dst->frame[9] = src->flag;
	if(src->flag == FDATA){
		for(i = 0;i <4;i++){
			dst->frame[i+10] = src->payload.data.Next_hop[i];
		}
		for(i = 0;i < src->length-14;i++){
			dst->frame[i+14] = src->payload.data.data[i];
		}
	}else if(src->flag == FRIP){
		for(i=0;i<32;i++){					 
			if(src->payload.route[i].Dst[3]!=0){ 
				dst->frame[10+9*i] = 0;
				dst->frame[11+9*i] = 0;
				dst->frame[12+9*i] = 0;
				dst->frame[13+9*i] = src->payload.route[i].Dst[3];
				dst->frame[14+9*i] = 0;
				dst->frame[15+9*i] = 0;
				dst->frame[16+9*i] = 0;
				dst->frame[17+9*i] = src->payload.route[i].Next_hop[3];
				dst->frame[18+9*i] = src->payload.route[i].Metric;
			}
		}
	}else if(src->flag == FBEACON){
		dst->frame[10] = src->payload.beacon.ID_Network;
		dst->frame[11] = src->payload.beacon.ID_Slot;

		tmp = (uint32_t ) src->payload.beacon.Voisin;
		dst->frame[12] = (uint8_t) ( tmp/(255*255*255) );
		dst->frame[13] = (uint8_t) ((tmp%(255*255*255))/(255*255));
		dst->frame[14] = (uint8_t) ((tmp%(255*255))/255);
		dst->frame[15] = (uint8_t) ( tmp%255 );

/*
		if(tmp >= s3){
				print("fault 1 \n\r");
			dst->frame[12] = (uint8_t) ( tmp/(255*255*255) );
			dst->frame[13] = (uint8_t) ((tmp%(255*255*255))/(255*255));
			dst->frame[14] = (uint8_t) ((tmp%(255*255))/255);
			dst->frame[15] = (uint8_t) ( tmp%255 );
		}else if(tmp >= s2){
				print("fault 2 \n\r");
			dst->frame[12] = 0;
			dst->frame[13] = (uint8_t) ((tmp%(255*255*255))/(255*255));
			dst->frame[14] = (uint8_t) ((tmp%(255*255))/255);
			dst->frame[15] = (uint8_t) ( tmp%255 );

		}else if(tmp >= s1){
				print("great 3 \n\r");
			dst->frame[12] = 0;
			dst->frame[13] = 0;
			dst->frame[14] = (uint8_t) ((tmp%(255*255))/255);
			dst->frame[15] = (uint8_t) ( tmp%255 );
		}else{
				print("fault 4 \n\r");
			dst->frame[12] = 0;
			dst->frame[13] = 0;
			dst->frame[14] = 0;
			dst->frame[15] = (uint8_t) ( tmp%255 );
		}		
 
		if(DEBUG){
			print("split :\n\r");
			ss[0] = dst->frame[12]/100 + '0' ;
			ss[1] = dst->frame[12]%100/10 + '0' ;
			ss[2] = dst->frame[12]%10 + '0' ;

			ss[3] = dst->frame[13]/100 + '0' ;
			ss[4] = dst->frame[13]%100/10 + '0' ;
			ss[5] = dst->frame[13]%10 + '0' ;

			ss[6] = dst->frame[14]/100 + '0' ;
			ss[7] = dst->frame[14]%100/10 + '0' ;
			ss[8] = dst->frame[14]%10 + '0' ;

			ss[9] = dst->frame[15]/100 + '0' ;
			ss[10] = dst->frame[15]%100/10 + '0' ;
			ss[11] = dst->frame[15]%10 + '0' ;

			print(ss);
			print("\n\r");
		}
*/
	}	
}

/*
*	transform the mrfiPacket to the struct for recieving
*/
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
	if(dst->flag == FDATA){
		for(i = 0;i <4;i++){
			dst->payload.data.Next_hop[i] = src->frame[i+10];
		}		
		for(i = 0;i < src->frame[0]-14;i++){
			dst->payload.data.data[i] = src->frame[i+14];
		}
	}else if(dst->flag == FRIP){
		for(i=0;i<(src->frame[0]-10)/9;i++){		
			dst->payload.route[i].Dst[3] = src->frame[13+9*i];
			dst->payload.route[i].Next_hop[3] = src->frame[17+9*i];
			dst->payload.route[i].Metric = src->frame[18+9*i];;
		}
	}else if(dst->flag == FBEACON){
		dst->payload.beacon.ID_Network = src->frame[10];
		dst->payload.beacon.ID_Slot = src->frame[11];
		dst->payload.beacon.Voisin = src->frame[12]*255*255*255 + src->frame[13]*255*255 +src->frame[14]*255 + src->frame[15];
	}	
}

/*
*	send beacon 
*/
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
//	MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_CCA);
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

//	print(output);		 
}

/*
*	send rip 
*/
void Send_rip(Status * s){			
	uint8_t i, cnt=0;

	mrfiPacket_t PacketToSend;
	mPacket Packet;

	Packet.src[3] = s->MAC;
	Packet.dst[3] = BROADCAST;
	//fill in the beacon flag
	Packet.flag  = FRIP; 

	for(i=0;i<32;i++){					 
		if(s->Route_table[i].Dst[3]!=0){ 
			Packet.payload.route[cnt].Dst[3] = s->Route_table[i].Dst[3];
			Packet.payload.route[cnt].Next_hop[3] = s->Route_table[i].Next_hop[3];
			Packet.payload.route[cnt].Metric = s->Route_table[i].Metric;
			cnt++;
		}
	}
	for(i=cnt;i<32;i++){					//clear the other dst 
		Packet.payload.route[i].Dst[3] = 0;
	}

	Packet.length =  10 + 9*cnt;
	SendmPacket(&Packet, &PacketToSend);

	if(cnt!=0){
		//send the rip
		//MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_CCA);
		MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_FORCED);
	}
}

/*
*	read the message in the FIFO_Recieve and find if there is '\r' , if yes then print 
*/
void Recieve_message(Status * s, QList *p){	//Recieve the message
	char output[MRFI_MAX_FRAME_SIZE-10]="";
	uint8_t i ,k;
	char c;

	i = 0;
	if(!IsEmpty(p)){
		output[i++] = p->front->data/10 + '0';		
		output[i++] = p->front->data%10 + '0';		
		output[i++] = ':';		
		while(Search(p, '\r')){				 
			c = DeQueue(p);
			if( c == '\r' ){
				output[i] = '\n';		
				output[i+1] = '\r';		
				print(output);

				i = 0;
				for(k=0;k<MRFI_MAX_FRAME_SIZE-10;k++){
					output[k]=0;
				}
				output[i++] = p->front->data/10 + '0';		
				output[i++] = p->front->data%10 + '0';	
				output[i++] = ':';		
			}else{
				output[i] = c;	
				i++;	
			}
		}
	}
}

/*
*	write the message to the FIFO_Send with the adr 
*/
void Send_message(Status * s, QList *Q, uint8_t  dst){	//send the message

	mrfiPacket_t PacketToSend;
	mPacket Packet;

	uint8_t i ,c ;
	char output[10] = "";		//for test

	if(!IsEmpty(Q)){
		Packet.src[3] = s->MAC;
		Packet.dst[3] = dst;
		Packet.flag = FDATA; 
	
		if(Length(Q)>=MRFI_MAX_FRAME_SIZE-14){
			Packet.length = MRFI_MAX_FRAME_SIZE;
			for(i = 0;i<MRFI_MAX_FRAME_SIZE-14;i++){
				c = DeQueue(Q);
				Packet.payload.data.data[i] = c ;
				//output[i] = c ;
			}
		}else{
			i = 0;
			Packet.length = Length(Q) + 14;
			while(!IsEmpty(Q)){
				c = DeQueue(Q);
				Packet.payload.data.data[i] = c ;
				//output[i] = c ;
				i++;
			}
		}

		//find the next hop
		if(Is_voisin(s,dst)){
			Packet.payload.data.Next_hop[3] = dst;
		}else{
			Packet.payload.data.Next_hop[3] = Find_next_hop(s , dst);
		}

		SendmPacket(&Packet, &PacketToSend);

		//send the message
		//MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_CCA);
		MRFI_Transmit(&PacketToSend, MRFI_TX_TYPE_FORCED);
	}
}

/*
*	sleeo , jump the mode LPM3
*/
void Sleep(void ){
	__bis_SR_register(LPM3_bits + GIE);       // Enter LPM0 w/ interrupt
}
