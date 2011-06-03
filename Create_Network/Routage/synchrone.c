#include "common.h"
#include <mrfi.h> 
#include <string.h> 
#include "interrupt.h"
#include "uart.h" 
#include "cycle.h" 
#include "stdio.h" 
#include "stdlib.h"
#include "synchrone.h"
#include "route.h"


Status etat;			//record all the status
extern uint8_t UART_MODE;

//char Message[4][20] = {"hello Node1","hello Node2","hello Node3","hello Node4",};

void Delay_Rand(uint32_t mod){			//wait for 0 ~ 100 ms
	volatile uint32_t x;
	x = rand()%mod;
	while(x--);
}

void delay_reste_slot(){			//5*n + 6
	volatile uint16_t x = 2277-500;
	while(x--);
}

void Synchrone_Init(uint8_t mac){
	etat.state = WAIT_SCAN;			//first time ;initialisation
	etat.ID_Network = NO_NETWORK;			//no network at first
	etat.MAC = mac;
	etat.HOST = IS_NOT_CREATER ;
	etat.synchrone = 0;
	etat.ID_Beacon = 0;
	etat.Dst = 0;
	etat.Counter = 0;
	etat.Surveille_Cnt = 0; 			 
	etat.Surveille_Cnt_Old = 0;
	Init_voisin(&etat);			//reset the table of route
	Init_route_table(&etat);
}


void Timer_Surveille(void);
interrupt(TIMERA0_VECTOR) Timer_Surveille(void)
{
	//Init_voisin(&etat);			//reset the table of route
	//Init_route_table(&etat);

	if(etat.HOST == IS_NOT_CREATER){
		if(etat.Surveille_Cnt != etat.Surveille_Cnt_Old){	 
			etat.Surveille_Cnt_Old = etat.Surveille_Cnt;
		}else{
			Delay_Rand(65535);			//delay some time
	 		etat.ID_Network = etat.MAC;
	  		etat.HOST = IS_CREATER;
			etat.synchrone = 1;
			etat.ID_Beacon = 0;
	 		etat.state = WAIT_SYNCHRONE;		//change the state
			timer_synchrone(&etat);
			Send_beacon(&etat);
			P1OUT |= 0x02;   			//jaune led
		}
	} 
}

void Timer_B0(void);
interrupt(TIMERB0_VECTOR) Timer_B0(void)
{
	etat.Counter--;
	if(etat.Counter == 0){
		Stop_Timer();
		if(etat.state == WAIT_SCAN && etat.ID_Network == NO_NETWORK){
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
					if(etat.HOST == IS_NOT_CREATER){
						etat.state = WAIT_SYNCHRONE;
						if(etat.ID_Beacon < etat.MAC){
							//timer_synchrone(&etat);
							P1OUT ^= 0x01;   			//rouge led
							Send_beacon(&etat);			//send beacon
							timer_synchrone(&etat);
						}else{			
							Stop_Timer();	
							etat.synchrone = 0;
							P1OUT ^= 0x01;   			//rouge led
							Send_beacon(&etat);			//send beacon
						}
					}			
					break;
				case WAIT_SYNCHRONE : 
					etat.state = WAIT_MESSAGE;
					//time for message
					timer_message(&etat);	

					Send_message(&etat, &FIFO_Send ,etat.Dst);
					Recieve_message(&etat, &FIFO_Recieve);

					break;
				case WAIT_MESSAGE :
					etat.state = WAIT_SLEEP;
					timer_sleep(&etat);
					//time for sleep
					//MRFI_RxIdle();		
					Sleep();
					break;
				case WAIT_SLEEP :
					__bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)	
					//MRFI_RxOn();				
					if(etat.HOST == IS_NOT_CREATER ){
						etat.state = WAIT_BEACON;
						if(etat.ID_Beacon < etat.MAC){
							Stop_Timer();
							etat.synchrone = 0;
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

void print(char *s){
	TXString(s, strlen(s));
}

void Buttopn(void);
interrupt(PORT1_VECTOR) Buttopn(void)
{
	P1IFG &= ~0x04;
	P1OUT ^=  0x03;
	//after press the button , we can send and recieve message 
	BSP_Init();
	MRFI_Init();
	MRFI_SetLogicalChannel(1);
	MRFI_SetRFPwr(0);

	Uart_Init();

	MRFI_WakeUp();
	MRFI_RxOn(); 

	Scan_Init(&etat);			//open timer B for scan
	Start_Timer_Surveille();		//open timer for surveille

	print(" bienvenu, vous puvez chat avec d'autre \n\r");
	print(" command help: \n\r");
	print(" s: show who is on line ,and choose one XX \n\r");
	print(" r: show router table \n\r");
	print(" c: start to chat \n\r");
}

void MRFI_RxCompleteISR()
{
	mrfiPacket_t PacketRecieved;
	mPacket Packet;
	char rssi ;
	char ss[9] = "";
	char dest[3] = "";

	uint8_t ID_Network_tmp, ID_Beacon_tmp ,src ,i ,data;
	uint32_t voisin_voisin;					//the voisin of voisin

	char output[MRFI_MAX_FRAME_SIZE-10]="";
	etat.Surveille_Cnt = ( etat.Surveille_Cnt + 1 )%65535;

	MRFI_Receive(&PacketRecieved);
	RecievemPacket(&PacketRecieved ,&Packet);

	if(Packet.flag == FBEACON){	
		ID_Network_tmp = Packet.payload.beacon.ID_Network;
		ID_Beacon_tmp  = Packet.payload.beacon.ID_Slot;
		src  = Packet.src[3];
		voisin_voisin = Packet.payload.beacon.Voisin;
		rssi = PacketRecieved.rxMetrics[0]; 
		if(rssi>-70){			//seuil avec pwr(0) ,distance 20cm
			Add_router(&etat , src, voisin_voisin);	//every time it recieve the beacon , update the route table
		}else{
			if(Is_voisin(&etat,src)){
				Delete_voisin(&etat, src);
				Delete_router(&etat , src);
			}
		}
/*
	ss[0] = etat.Route_table[0].Dst[3];	 
	ss[1] = etat.Route_table[0].Next_hop[3];
	ss[2] = etat.Route_table[0].Metric;
	ss[3] = etat.Route_table[2].Dst[3];	 
	ss[4] = etat.Route_table[2].Next_hop[3];
	ss[5] = etat.Route_table[2].Metric;
	ss[6] = etat.Route_table[4].Dst[3];	 
	ss[7] = etat.Route_table[4].Next_hop[3];
	ss[8] = etat.Route_table[4].Metric;
	print(ss);
*/
		if(etat.synchrone == 0  && etat.HOST == IS_NOT_CREATER){	
			Stop_Timer();
			etat.ID_Network = ID_Network_tmp;		 
			etat.ID_Beacon  = ID_Beacon_tmp;
			etat.synchrone = 1;
			if(etat.MAC > etat.ID_Beacon ){
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
			Stop_Timer();
			etat.ID_Network = ID_Network_tmp;		 
			etat.ID_Beacon  = ID_Beacon_tmp;
			etat.synchrone = 1;
			if(etat.MAC > etat.ID_Beacon ){
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

		//print(output);

	}else if(Packet.flag == FDATA){
		if(Packet.payload.data.Next_hop[3] == etat.MAC){ 	//if next hop is him, relay and change the next hop
			if(Packet.dst[3] == etat.MAC){			//if it is really for me
				etat.Dst = Packet.src[3];
				FIFO_Recieve.front->data = etat.Dst;
				FIFO_Send.front->data = etat.Dst;
				if(UART_MODE!=2){			//change mode to type if recieve a paquet
					UART_MODE = 2;
					print("\n\r Recieved message from ");
					dest[0] = etat.Dst/10 + '0';
					dest[1] = etat.Dst%10 + '0';
					print(dest);
					print("\n\r");
				}
				for (i=0;i<Packet.length-14;i++) {
					data = Packet.payload.data.data[i];
					EnQueue(&FIFO_Recieve,data);
					//EnQueue(&FIFO_Send,data);
				}
			}else{						//if it just nedd relay
				P1OUT ^= 0x02; 
				PacketRecieved.frame[13] = Find_next_hop(&etat , Packet.dst[3]);
				MRFI_Transmit(&PacketRecieved, MRFI_TX_TYPE_CCA);
				//MRFI_Transmit(&PacketRecieved, MRFI_TX_TYPE_FORCED);	
			}
		}
		if(Packet.dst[3] == BROADCAST){				//if it is message broadcast
			for (i=0;i<Packet.length-14;i++) {
				data = Packet.payload.data.data[i];
				EnQueue(&FIFO_Recieve,data);
			}
		}
	}else if(Packet.flag == FRIP){
		;//a completer
	}
}


