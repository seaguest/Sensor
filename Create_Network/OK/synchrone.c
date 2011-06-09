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

/*
*	the synchronisation is updated when sensor gets a packet of beacon
*	the synchronisation is redone every duty cycle
*	when it gets a packet ,check the flaf then deals with it 
*/


 Status etat;					//record all the status
extern uint8_t UART_MODE;
volatile uint8_t RIP_Prepared = 0;

/*
*	when find the network is down then after certain time then create another
*/
void Delay_Rand(uint32_t mod){			//wait for 0 ~ 100 ms
	volatile uint32_t x;
	x = rand()%mod;
	while(x--);
}

/*
*	initialisation for synchronisation
*/
void Synchrone_Init(uint8_t mac){
	uint8_t i;

	etat.state = WAIT_SCAN;			//first time ;initialisation
	etat.ID_Network = NO_NETWORK;		//no network at first
	etat.MAC = mac;
	etat.HOST = IS_NOT_CREATER ;
 	etat.synchrone = 0;			 

	etat.ID_Beacon = 0;
	etat.Dst = 0;
	etat.Counter = 0;
	etat.Surveille_Cnt = 0; 			 
	etat.Surveille_Cnt_Old = 0;

	InitQueue(&etat.FIFO_Send);
	InitQueue(&etat.FIFO_Recieve);

	Init_voisin(&etat);
	Init_route_table(&etat);		//reset the table of route

	for(i = 0; i<N_SLOT-2; i++){
		etat.check_old[i] = etat.check[i] = 0;
	}
}


/*
*	interruption of button
*	initialisation for scanning ,uart , cc2500 and timers  

*/
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

	print("\n\r");
	print("command: \n\r");
	print("o  : who is on line \n\r");
	print("v  : voisin \n\r");
	print("r  : router table \n\r");
	print("i  : sysinfo \n\r");
	print("ESC: help \n\r");
}

/*
*	interruption of timer A 
*	maintenance of network and update router table
*/
void Timer_Surveille(void);
interrupt(TIMERA0_VECTOR) Timer_Surveille(void)
{
	uint8_t i, j;
	RIP_Prepared = 1;			//signal for rip, after this it has the right to send rip
	Start_Timer_Surveille();		//open timer for surveille

	for(i = 0; i<N_SLOT-2; i++){
		if(Is_voisin(&etat, i+1) && i != (etat.MAC-1)){		
			if(etat.check[i] != etat.check_old[i]){		//if always recieve beacon of voisin 	 
				etat.check_old[i] = etat.check[i];
			}else{					//if not ,i +1 disappered
				etat.check_old[i] = etat.check[i] = 0;
				Delete_voisin(&etat, i+1);
				Delete_router(&etat, i+1);
			}
		}
	}

	if(etat.HOST == IS_NOT_CREATER){	//if not the HOST
		if(etat.Surveille_Cnt != etat.Surveille_Cnt_Old){//if the network is OK , contuinue	 
			etat.Surveille_Cnt_Old = etat.Surveille_Cnt;
		}else{				//if the network is down
			Delay_Rand(65535);			//delay some time
	 		etat.ID_Network = etat.MAC;
	  		etat.HOST = IS_CREATER;
		 	etat.synchrone = 1;			 
			etat.ID_Beacon = 0;
			etat.Surveille_Cnt = etat.Surveille_Cnt_Old = 0;
			etat.state = WAIT_SYNCHRONE;
			timer_synchrone(&etat);
			Send_beacon(&etat);
			Init_voisin(&etat);	
			Init_route_table(&etat);		//reset the table of route

			for(j = 0; j<N_SLOT-2; j++){
				etat.check_old[j] = etat.check[j] = 0;
			}

			P1OUT |= 0x02;   			//jaune led
		}
	} 
}

/*
*	interruption of timer B
*	synchronisation and every part of duty cycle
*	a automate for the duty cycle 
*/
void Timer_B0(void);
interrupt(TIMERB0_VECTOR) Timer_B0(void)
{
	etat.Counter--;
	if(etat.Counter == 0){
		Start_Timer_Surveille();		//the timer A0 is closed sometime, very strange	
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
						etat.state = WAIT_SYNCHRONE;	//change the state
						if(etat.ID_Beacon < etat.MAC){
							timer_synchrone(&etat);
							P1OUT ^= 0x01;   			//rouge led
							Send_beacon(&etat);			//send beacon
						}else{			
							Stop_Timer();	
						}
					}
					break;
				case WAIT_SYNCHRONE : 
					etat.state = WAIT_MESSAGE;	//change the state
					//time for message
					timer_message(&etat);	
					if(etat.HOST == IS_NOT_CREATER ){
					 	etat.synchrone = 0;			 
					}

					if(etat.Dst != 0){
						Send_message(&etat, &etat.FIFO_Send ,etat.Dst);
					}
					Recieve_message(&etat, &etat.FIFO_Recieve);

					if(RIP_Prepared == 1){			//every 3s ,send the rip
						Tidy_table(&etat);		// clear the dirty data
						Send_rip(&etat);
						RIP_Prepared = 0;
					}
					break;
				case WAIT_MESSAGE :
					etat.state = WAIT_SLEEP;	//change the state
					//time for sleep
					timer_sleep(&etat);
					Sleep();
					break;
				case WAIT_SLEEP :
					if(etat.HOST == IS_NOT_CREATER ){
						etat.state = WAIT_BEACON;	//change the state

						if(etat.ID_Beacon < etat.MAC){
							Stop_Timer();
						}else{
							timer_send_beacon(&etat);
						}
					}else{						//the host
						etat.state = WAIT_SYNCHRONE;
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


/*
*	interruption of recieving packet
*	trait the packet with the flag
*/
void MRFI_RxCompleteISR()
{
	mrfiPacket_t PacketRecieved;
	mPacket Packet;
	char rssi ;

	uint8_t ID_Network_tmp, ID_Beacon_tmp ,src ,i ,data;
	uint32_t voisin_voisin;					//the voisin of voisin

	MRFI_Receive(&PacketRecieved);
	RecievemPacket(&PacketRecieved ,&Packet);

	if(Packet.flag == FBEACON){	
		etat.Surveille_Cnt = ( etat.Surveille_Cnt + 1 )%65535;

		ID_Network_tmp = Packet.payload.beacon.ID_Network;
		ID_Beacon_tmp  = Packet.payload.beacon.ID_Slot;
		src  = Packet.src;
		voisin_voisin = Packet.payload.beacon.Voisin;
		rssi = PacketRecieved.rxMetrics[0]; 
		if(rssi > -68){			//seuil avec pwr(0) ,distance 20cm
			etat.check[src-1] = (etat.check[src-1] + 1)%65535;	//make sure if the voisin is there
			Add_router(&etat , src, voisin_voisin);		//every time it recieve the beacon , update the route table
		}
/*
		print_8b(ID_Network_tmp);
		print_8b(ID_Beacon_tmp);
		print(" ");
		print_8b(-rssi);
		print("\n\r");
*/

		if(rssi < -84 && etat.ID_Beacon == ID_Beacon_tmp && etat.ID_Beacon !=0 && etat.HOST == IS_NOT_CREATER){ //if the beacon source go far , choose another
			Stop_Timer();
		}else{
			if(etat.synchrone == 0 && etat.HOST == IS_NOT_CREATER){	
				Stop_Timer();
				etat.ID_Network = ID_Network_tmp;		 
				etat.ID_Beacon  = ID_Beacon_tmp;
			 	etat.synchrone = 1;			 

				if(etat.MAC > etat.ID_Beacon ){
					etat.state = WAIT_BEACON ;	//change the state
					timer_send_beacon(&etat);	
				}else{
					etat.state = WAIT_SYNCHRONE ;	//change the state
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
					etat.state = WAIT_BEACON;	//change the state
					timer_send_beacon(&etat);	
				}else{
					etat.state = WAIT_SYNCHRONE ;	//change the state
					timer_synchrone(&etat);
				}			
			}
		}
	}else if(Packet.flag == FDATA){
		if(Packet.payload.data.Next_hop == etat.MAC){ 	//if next hop is him, relay and change the next hop
			if(Packet.dst == etat.MAC){			//if it is really for me
				etat.Dst = Packet.src;
				if(UART_MODE!=2){			//change mode to type if recieve a paquet
					UART_MODE = 2;
					print("\n\rRecieved message from: ");
					print_8b(etat.Dst);
					print("\n\r");
				}
				for (i=0;i<Packet.length-11;i++) {
					data = Packet.payload.data.data[i];
					EnQueue(&etat.FIFO_Recieve,data);
				}
			}else{						//if it just nedd relay
				P1OUT ^= 0x02; 
				PacketRecieved.frame[10] = Find_next_hop(&etat , Packet.dst);
				MRFI_Transmit(&PacketRecieved, MRFI_TX_TYPE_CCA);
				//MRFI_Transmit(&PacketRecieved, MRFI_TX_TYPE_FORCED);	
			}
		}
		if(Packet.dst == BROADCAST){				//if it is message broadcast
			for (i=0;i<Packet.length-11;i++) {
				data = Packet.payload.data.data[i];
				EnQueue(&etat.FIFO_Recieve,data);
			}
		}
	}else if(Packet.flag == FRIP){		//recieve the packet of rip then update the router table
		Update_rip(&etat ,&Packet);
	}
}


