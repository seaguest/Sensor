
/***************************************************************************************
Copyright (C), 2011-2012, ENSIMAG.TELECOM 
File name	: route.c
Author		: HUANG yongkan & KANJ mahamad
Version		:
Date		: 2011-6-6
Description	: we use a 32b for recording the voisin 
		  then we use protocol RIP for sharing the router table
		  here are the functions for manipulations of router table 
Function List	:  
		  uint32_t puissance(uint8_t i);					// calcul 2^i
		  void Init_voisin(volatile Status *s);					// initialisation for 32b of voisin , put itself in
		  void Add_voisin(volatile Status *s, uint8_t id);			// add id as its voisin
		  void Delete_voisin(volatile Status *s, uint8_t id);			// delete the voisin id
		  uint8_t Is_voisin(volatile Status *s, uint8_t dst);			// check if dst is voisin
		  uint8_t Default_GW(volatile Status *s);				// choose the default gateway	
		  void Init_route_table(volatile Status *s);				// initialisation for router table
		  void Add_router(volatile Status *s , uint8_t id, uint32_t voisin);	// add router
		  uint8_t Find_next_hop(volatile Status *s , uint8_t dst);		// for a dst ,find next hop in the router table 
		  void Delete_router(volatile Status *s , uint8_t id);			// delete the router
		  void Show_Online(volatile Status *s);					// print the sensors on line
		  void Show_router(volatile Status *s);					// print the router table
		  uint8_t Find_index(uint8_t id ,mPacket *m);				// find index in rip packet
		  void Update_rip(volatile Status *s ,mPacket *m);			// update router table with rip
		  void Show_voisin(volatile Status *s);					// print voisin	
		  void Tidy_table(volatile Status *s);					// clear dirty data in router table	 
		  void Set_filteredtable(volatile Status *s);				// set filter
***************************************************************************************/

#include"route.h"
#include"uart.h"
#include <string.h> 

 
/***************************************************************************************
Function	: uint32_t puissance(uint8_t i)
Description	: calcul 2^i  (16bit mcu dont support 1<<32 very well)
Calls		:  
Called By	: void Add_voisin(volatile Status *s, uint8_t id)
		  void Add_voisin(volatile Status *s, uint8_t id)
		  void Delete_voisin(volatile Status *s, uint8_t id)
		  uint8_t Is_voisin(volatile Status *s, uint8_t dst)
		  uint8_t Default_GW(Status *s)
		  void Add_router(volatile Status *s , uint8_t id, uint32_t voisin
Input		: void
Output		: 
Return		: 2^i
Others		: 
***************************************************************************************/

uint32_t puissance(uint8_t i)
{ 
	uint32_t tmp;
	tmp = 0;
	if(i < 15){
		return (uint32_t ) (1 << i);
	}else{
		tmp = (uint32_t ) (1 << 14);
		return (uint32_t )(1 << (i - 14))*tmp;
	}
}


/***************************************************************************************
Function	: void Init_voisin(volatile Status *s)
Description	: initialisation for 32b of voisin , put itself in
Calls		:  
Called By	: interrupt(TIMERA0_VECTOR) Timer_Surveille(void)
		  void Synchrone_Init(uint8_t mac)
Input		: volatile Status *s
Output		: 
Return		: void
Others		: 
***************************************************************************************/

void Init_voisin(volatile Status *s)
{
	s->Voisin = 0;	
	s->Voisin |= (uint32_t ) puissance(s->MAC - 1);
}

/***************************************************************************************
Function	: void Add_voisin(volatile Status *s, uint8_t id)
Description	: add id as its voisin
Calls		:  
Called By	: void Add_router(volatile Status *s , uint8_t id, uint32_t voisin)
Input		: volatile Status *s, uint8_t id
Output		: 
Return		: void
Others		: 
***************************************************************************************/

void Add_voisin(volatile Status *s, uint8_t id)
{
	s->Voisin |= (uint32_t ) puissance(id-1);
}

/***************************************************************************************
Function	: void Delete_voisin(volatile Status *s, uint8_t id)
Description	: delete the voisin id
Calls		:  
Called By	: interrupt(TIMERA0_VECTOR) Timer_Surveille(void)
Input		: volatile Status *s
Output		: delete voisin id
Return		: void
Others		: 
***************************************************************************************/

void Delete_voisin(volatile Status *s, uint8_t id)
{
	s->Voisin &= (uint32_t ) (puissance(id-1)^0xFFFFFFFF);
}

/***************************************************************************************
Function	: uint8_t Is_voisin(volatile Status *s, uint8_t dst)
Description	: check if dst is its voisin
Calls		:  
Called By	: 
Input		: volatile Status *s , uint8_t dst
Output		: 
Return		: 1  if dst is voisin
		  0 if dst is not voisin
Others		: 
***************************************************************************************/

uint8_t Is_voisin(volatile Status *s, uint8_t dst)
{		
	uint32_t  tmp;
	tmp = puissance(dst-1);
	if((s->Voisin&tmp)==tmp){
		return 1;
	}else{
		return 0;
	}
}

/***************************************************************************************
Function	: uint8_t Default_GW(Status *s)
Description	: return the default gateway , we choose the first one and not itself or return broadcast
Calls		:  
Called By	: 
Input		: volatile Status *s
Output		: 
Return		: void
Others		: 
***************************************************************************************/

/*
uint8_t Default_GW(Status *s)
{				
	uint32_t  i ,tmp;
	for(i=0;i<30;i++){
		tmp = puissance(i);
		if((s->Voisin&tmp)==tmp && i!=(s->MAC-1)){
			return i+1;
		}
	}
	return BROADCAST;
}
*/

/***************************************************************************************
Function	: void Init_route_table(volatile Status *s)
Description	: initialisation for router table
Calls		:  
Called By	: 
Input		: volatile Status *s
Output		: 
Return		: void
Others		: 
***************************************************************************************/

void Init_route_table(volatile Status *s)
{
	uint8_t i;
	for(i = 0;i<N_SLOT;i++){
		if(i == (s->MAC-1)){		//itself
			s->Route_table[i].Dst = s->MAC;	 
			s->Route_table[i].Next_hop = s->MAC;
			s->Route_table[i].Metric = 0;
		}else{
			s->Route_table[i].Dst = 0;	 
			s->Route_table[i].Next_hop = 0;
			s->Route_table[i].Metric = 255;
		}
	}
}

/***************************************************************************************
Function	: void Delete_router(volatile Status *s , uint8_t id)
Description	: delete id in the router table
Calls		:  
Called By	: interrupt(TIMERA0_VECTOR) Timer_Surveille(void)
Input		: volatile Status *s , uint8_t id
Output		: delete the dst depends on id
Return		: void
Others		: 
***************************************************************************************/

void Delete_router(volatile Status *s , uint8_t id)
{
	uint8_t i;
	for(i=0;i<30;i++){				//delete all nodes who need id as next hop	 
		if(s->Route_table[i].Next_hop == id){
			s->Route_table[i].Dst = 0;	 
			s->Route_table[i].Next_hop = 0;
			s->Route_table[i].Metric = 255;	
		}
	}
}

/***************************************************************************************
Function	: void Add_router(volatile Status *s , uint8_t id, uint32_t voisin)
Description	: get the information of its voisin and update its router table
		  after recieving the beacon , update the route table
Calls		:  
Called By	: void MRFI_RxCompleteISR()
Input		: volatile Status *s , uint8_t id, uint32_t voisin
Output		: 
Return		: void
Others		: 
***************************************************************************************/

void Add_router(volatile Status *s , uint8_t id, uint32_t voisin)
{	
	uint8_t i;
	uint32_t tmp;

	if(!Is_voisin(s,id)){					//only listen to voisin 
		Add_voisin(s,id);
		s->Route_table[id-1].Dst = id;	 
		s->Route_table[id-1].Next_hop = id;	 
		s->Route_table[id-1].Metric = 1;
	}
	for(i=0;i<30;i++){					 
		tmp = puissance(i);
		if((voisin&tmp)==tmp &&  !Is_voisin(s,i+1)){	//if it is not my voisin ; add it
			s->Route_table[i].Dst = i+1;	 
			s->Route_table[i].Next_hop = id;	 
			s->Route_table[i].Metric = 2;
		}
	}
}


/***************************************************************************************
Function	: uint8_t Find_next_hop(volatile Status *s , uint8_t dst)
Description	: find th next hop for a certain dst
Calls		:  
Called By	: void Send_message(volatile Status * s, volatile QList *Q, uint8_t  dst)
		  void MRFI_RxCompleteISR()
Input		: volatile Status *s , uint8_t dst
Output		: 
Return		: the next_hop
Others		: 
***************************************************************************************/

uint8_t Find_next_hop(volatile Status *s , uint8_t dst)
{		
	if(s->Route_table[dst-1].Dst == 0){			// it is not in route table
		return BROADCAST;//Default_GW(s);
	}else{
		return s->Route_table[dst-1].Next_hop;	//find the next hop
	}
}

/***************************************************************************************
Function	: uint8_t Find_index(uint8_t id ,mPacket *m)
Description	: find the index which correspond certain id in the rip packet
Calls		:  
Called By	: void Update_rip(volatile Status *s ,mPacket *m)
Input		: uint8_t id ,mPacket *m
Output		: 
Return		: i  if in the packet of rip find the id ,return index	
		  32 if in the packet of rip ; there is not such id 		
Others		: 
***************************************************************************************/

uint8_t Find_index(uint8_t id ,mPacket *m)
{			
	uint8_t i;
	for(i = 0;i<((m->length-10)/3);i++){
		if(m->payload.route[i].Dst == id){
			return i;
		}
	}
	return 32;				//if failed, not exist
}


/***************************************************************************************
Function	: void Update_rip(volatile Status *s ,mPacket *m)
Description	: with the rip recieved , then update the router table
Calls		:  
Called By	: void MRFI_RxCompleteISR()
Input		: volatile Status *s ,mPacket *m
Output		: update the router table
Return		: void
Others		: 
***************************************************************************************/

void Update_rip(volatile Status *s ,mPacket *m)
{
	uint8_t i ,src, index;
	src = m->src;
	if(Is_voisin(s,src)){		//only listen to voisin
		for(i = 0;i<30;i++){
			if(s->Route_table[i].Dst==0 &&((index=Find_index(i+1,m))!=32 && s->Route_table[i].Next_hop != s->MAC)){
				s->Route_table[i].Dst = i+1;	 
				s->Route_table[i].Next_hop = src; 
				s->Route_table[i].Metric = m->payload.route[index].Metric +1;
			}else if(s->Route_table[i].Dst!=0 && (Find_index(i+1,m)==32) && s->Route_table[i].Next_hop == src ){
				s->Route_table[i].Dst = 0;	 
				s->Route_table[i].Next_hop = 0; 
				s->Route_table[i].Metric = 255;
			}
		}
	}
}

/***************************************************************************************
Function	: void Set_filteredtable(volatile Status *s)
Description	: set the adress for filtering
Calls		:  
Called By	: 
Input		: volatile Status *s
Output		: 
Return		: void
Others		: 
***************************************************************************************/

/*
void Set_filteredtable(volatile Status *s)
{
	uint8_t i, a;
	MRFI_DisableRxAddrFilter();
	for(i=0;i<30;i++){			
		if(!Is_voisin(s, i+1)){		//if it is not voisin, put it in the filter table
			a = i+1;
			MRFI_SetRxAddrFilter(&a);
		}		
	}
	MRFI_EnableRxAddrFilter();
}
*/


/***************************************************************************************
Function	: void Tidy_table(volatile Status *s)
Description	: clear the dirty data in router table
Calls		:  
Called By	: interrupt(TIMERB0_VECTOR) Timer_B0(void)
		  interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: volatile Status *s
Output		: clear the dirty data in router table
Return		: void
Others		: 
***************************************************************************************/

void Tidy_table(volatile Status *s)
{
	uint8_t i;
	for(i=0;i<30;i++){			
		if(s->Route_table[i].Dst!=0){ 
			if(s->Route_table[i].Next_hop > 30 || (s->Route_table[i].Metric > 15 && s->Route_table[i].Metric  != 255)){
				s->Route_table[i].Dst = 0;	 
				s->Route_table[i].Next_hop = 0; 
				s->Route_table[i].Metric = 255;
			}
		}
	}
}


/***************************************************************************************
Function	: void Show_Online(volatile Status *s)
Description	: print the sensors who are on line , reachable
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: volatile Status *s
Output		: print the sensors on line
Return		: void
Others		: 
***************************************************************************************/

void Show_Online(volatile Status *s)
{
	uint8_t i;
	print("\n\r");
	for(i=0;i<30;i++){					 
		if(s->Route_table[i].Dst!=0){ 
			print(" ");
			print_8b(i+1);
		}
	}
	print("\n\r");
}


/***************************************************************************************
Function	: void Show_voisin(volatile Status *s)
Description	: print the voisin
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: volatile Status *s
Output		: print the voisin
Return		: void
Others		: 
***************************************************************************************/

void Show_voisin(volatile Status *s)
{
	uint8_t i;
	print("Voisin is: \n\r");
	for(i=0;i<30;i++){		 
		if(Is_voisin(s,i+1)){
			print_8b(i+1);
			print(" ");
		}
	}			 
	print("\n\r");
}


/***************************************************************************************
Function	: void Show_router(volatile Status *s)
Description	: print the router table
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: volatile Status *s
Output		: print the router table
Return		: void
Others		: 
***************************************************************************************/

void Show_router(volatile Status *s)
{
	uint8_t i;
	print("\n\r router table : \n\r");
	for(i=0;i<30;i++){					 
		if(s->Route_table[i].Dst!=0){ 
 			print("Dst:");
 			print_8b(i+1);
 			print(" Next_hop:");
 			print_8b(s->Route_table[i].Next_hop);
 			print(" Metric:");
  			print_8b(s->Route_table[i].Metric);
 			print("\n\r");
		}
	}
 	print("Over! \n\r");
}




