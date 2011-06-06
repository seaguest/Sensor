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
uint8_t RIP_Prepared = 0;
uint8_t permission = 0;


void mutex(uint8_t *u ,uint8_t v ){
	while(permission){}
	permission = 1;
	*u = v;
	permission = 0;
}

/*
*	for the changing the value of etat.synchrone ,we should use semaphore to avoid collisions
*	we take the person algorithme
*/
//flag[2] is boolean; and turn is an integer
uint8_t flag[2]  = {0, 0};
uint8_t turn;

/*
*	Set_Synchrone  , in the ISR of RF
*/
void Set_Synchrone(void ){
	flag[0] = 1;
	turn = 1;
	while (flag[1] == 1 && turn == 1){}	//wait if the varible is busy 
	etat.synchrone = 1;			//section critique
	flag[0] = 0;
}

/*
*	clear_Synchrone , in the ISR of Timer
*/
void Clear_Synchrone(void ){
	flag[1] = 1;
	turn = 0;
	while (flag[0] == 1 && turn == 0){}	//wait if the varible is busy 
 	etat.synchrone = 0;			//section critique
	flag[1] = 0;
}

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
	etat.state = WAIT_SCAN;			//first time ;initialisation
	etat.ID_Network = NO_NETWORK;		//no network at first
	etat.MAC = mac;
	etat.HOST = IS_NOT_CREATER ;
 	etat.synchrone = 0;			//section critique

	etat.ID_Beacon = 0;
	etat.Dst = 0;
	etat.Counter = 0;
	etat.Surveille_Cnt = 0; 			 
	etat.Surveille_Cnt_Old = 0;
	Init_voisin(&etat);			
	Init_route_table(&etat);		//reset the table of route
}


/*
*	interruption of timer A 
*	maintenance of network and update router table
*/
void Timer_Surveille(void);
interrupt(TIMERA0_VECTOR) Timer_Surveille(void)
{
	RIP_Prepared = 1;			//signal for rip, after this it has the right to send rip
	if(etat.HOST == IS_NOT_CREATER){	//if not the HOST
		if(etat.Surveille_Cnt != etat.Surveille_Cnt_Old){//if the network is OK , contuinue	 
			etat.Surveille_Cnt_Old = etat.Surveille_Cnt;
		}else{				//if the network is down
			Delay_Rand(65535);			//delay some time
	 		etat.ID_Network = etat.MAC;
	  		etat.HOST = IS_CREATER;
		 	etat.synchrone = 1;			//section critique

			etat.ID_Beacon = 0;
			etat.state = WAIT_SYNCHRONE;
			timer_synchrone(&etat);
			Send_beacon(&etat);
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
/*
	if(Clock() == 1 && etat.state!=WAIT_SLEEP){	  // only sleep choose 1 ,vlo	 
		__bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)	
		TBCTL=TBSSEL_2 + MC_1;     		  
		TBCCTL0 = CCIE;                        
		TBCCR0 = (uint16_t) N_1MS;		 //delay 1ms 
	}
*/

	etat.Counter--;
	if(etat.Counter == 0){
		Stop_Timer();
		if(etat.state == WAIT_SCAN && etat.ID_Network == NO_NETWORK){
	 		etat.ID_Network = etat.MAC;
	  		etat.HOST = IS_CREATER;
		 	etat.synchrone = 1;			//section critique
			etat.state = WAIT_SYNCHRONE;	//change the state
			timer_synchrone(&etat);
			Send_beacon(&etat);
			P1OUT |= 0x02;   			//jaune led
	 	}else{
			switch(etat.state){
				case WAIT_BEACON : 
					//if(DEBUG){print("ok beacon is over \n\r"); }
					if(etat.HOST == IS_NOT_CREATER){
						etat.state = WAIT_SYNCHRONE;	//change the state
						if(etat.ID_Beacon < etat.MAC){
							timer_synchrone(&etat);
							//if(DEBUG){print("ok MAC > t_synchrone is set \n\r"); }
							P1OUT ^= 0x01;   			//rouge led
							Send_beacon(&etat);			//send beacon
						}else{			
							Stop_Timer();	
							//if(DEBUG){print("ok MAC < timer stoped \n\r"); }
						}
					}
					break;
				case WAIT_SYNCHRONE : 
					//if(DEBUG){print("ok synchrone is over \n\r");}
					etat.state = WAIT_MESSAGE;	//change the state
					//time for message
					timer_message(&etat);	
					//if(DEBUG){print("ok t_message is set \n\r"); }	
					if(etat.HOST == IS_NOT_CREATER ){
					 	etat.synchrone = 0;			//section critique
					}
/*
					Send_message(&etat, &FIFO_Send ,etat.Dst);
					Recieve_message(&etat, &FIFO_Recieve);
					if(RIP_Prepared == 1){		//every 3s ,send the rip
						Send_rip(&etat);
						RIP_Prepared = 0;
					}
*/
					break;
				case WAIT_MESSAGE :
					//if(DEBUG){print("ok message is over \n\r");}	
					etat.state = WAIT_SLEEP;	//change the state
					//time for sleep
					timer_sleep(&etat);
					//if(DEBUG){print("ok t_sleep is set \n\r");	}	

					if(etat.HOST == IS_NOT_CREATER ){
					 	etat.synchrone = 0;			//section critique
					}

					Sleep();
					//if(DEBUG){print("ok fall in sleep \n\r");	}	
					break;
				case WAIT_SLEEP :
					//if(DEBUG){print("ok sleep is over \n\r");	}	
					__bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)	
					if(etat.HOST == IS_NOT_CREATER ){
						etat.state = WAIT_BEACON;	//change the state

				 		//if(DEBUG){print("ok WAIT_BEACON change synchrone \n\r");	}		

						if(etat.ID_Beacon < etat.MAC){
							Stop_Timer();
				 			//if(DEBUG){print("ok MAC > ,stop timer \n\r");	}		
						}else{
							timer_send_beacon(&etat);
				 			//if(DEBUG){print("ok MAC <,t_send_beacon is set \n\r");	}		
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
	print("r  : router table \n\r");
	print("i  : sysinfo \n\r");
	print("ESC: help \n\r");
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
	char dest[3] = "";
	char ss[5] = "";

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
		if(rssi>-72){			//seuil avec pwr(0) ,distance 20cm
			Add_router(&etat , src, voisin_voisin);	//every time it recieve the beacon , update the route table
		}else{
			if(Is_voisin(&etat,src)){
				Delete_voisin(&etat, src);
				Delete_router(&etat , src);
			}
		}
	
		//if(DEBUG){print("ok got a beacon \n\r"); }		

		if(rssi < -84 && etat.ID_Beacon == ID_Beacon_tmp && etat.ID_Beacon !=0 && etat.HOST == IS_NOT_CREATER){ //if the beacon source go far , choose another
			Stop_Timer();
		}else{
			//P1OUT ^= 0x02;   			//rouge led
			if(etat.synchrone == 0 && etat.HOST == IS_NOT_CREATER){	
				Stop_Timer();
				etat.ID_Network = ID_Network_tmp;		 
				etat.ID_Beacon  = ID_Beacon_tmp;
			 	etat.synchrone = 1;			//section critique

				//if(DEBUG){print("ok same network RF change synchrone \n\r");}			

				if(etat.MAC > etat.ID_Beacon ){
					etat.state = WAIT_BEACON ;	//change the state
					timer_send_beacon(&etat);	
					//if(DEBUG){print("ok MAC > first syn ,t_send_beacon is set \n\r");}						
				}else{
					etat.state = WAIT_SYNCHRONE ;	//change the state
					timer_synchrone(&etat);
					//if(DEBUG){print("ok MAC < first syn ,t_synchrone is set \n\r");}						
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
			 	etat.synchrone = 1;			//section critique

				//if(DEBUG){print("ok new network  RF change synchrone \n\r");}			

				if(etat.MAC > etat.ID_Beacon ){
					etat.state = WAIT_BEACON;	//change the state
					timer_send_beacon(&etat);	
					//if(DEBUG){print("ok MAC > first syn ,t_send_beacon is set \n\r");}						
				}else{
					etat.state = WAIT_SYNCHRONE ;	//change the state
					timer_synchrone(&etat);
					//if(DEBUG){print("ok MAC < first syn ,t_synchrone is set \n\r");}						
				}			
			}
		}
		

/*
		//show the ID_NETWORK
		output[0] = ID_Network_tmp/10 + '0';
		output[1] = ID_Network_tmp%10 + '0';
		//show the MAC of the source
		output[2] = ID_Beacon_tmp/10 + '0';
		output[3] = ID_Beacon_tmp%10 + '0';

		print(output);

		if(rssi<0){rssi = -rssi;ss[0] = '-';}
		ss[1] = rssi/100 + '0';
		ss[2] = rssi%100/10 + '0';
		ss[3] = rssi%10 + '0';
		print(ss);
		print("\n\r");
*/

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
				//if(DEBUG){print("\n\r Message Recieved ok "); }
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
	}else if(Packet.flag == FRIP){		//recieve the packet of rip then update the router table
		Update_rip(&etat ,&Packet);
	}
}


