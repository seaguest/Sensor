#include"route.h"
#include"uart.h"
#include <string.h> 

/*
*	we use a 32b for recording the voisin 
*	then we use protocol RIP for sharing the router table
*	here are the functions for manipulations of router table 
*/


/*
*	calcule 2^i
*/
uint32_t puissance(uint8_t i){				
	uint32_t tmp= 1;
	while(i--){
		tmp *= 2;
	}
	return tmp;
}  

/*
*	initialisation for 32b of voisin , put itself in
*/
void Init_voisin(Status *s){
	s->Voisin = 0;	
	s->Voisin |= (uint32_t ) puissance(s->MAC - 1);
}

/*
*	add id as its voisin
*/
void Add_voisin(Status *s, uint8_t id){
	s->Voisin |= (uint32_t ) puissance(id-1);
}

/*
*	delete the voisin id
*/
void Delete_voisin(Status *s, uint8_t id){
	s->Voisin &= ((uint32_t ) puissance(id-1)^0xFFFFFFFF);
}

/*
*	check if dst is its voisin
*/
uint8_t Is_voisin(Status *s, uint8_t dst){		
	uint32_t  tmp;
	tmp = puissance(dst-1);
	if((s->Voisin&tmp)==tmp){
		return 1;
	}else{
		return 0;
	}
}

/*
*	return the default gateway , we choose the first one and not itself or return broadcast
*/
uint8_t Default_GW(Status *s){				
	uint32_t  i ,tmp;
	for(i=0;i<32;i++){
		tmp = puissance(i);
		if((s->Voisin&tmp)==tmp && i!=(s->MAC-1)){
			return i+1;
		}
	}
	return BROADCAST;
}

/*
*	initialisation for router table
*/
void Init_route_table(Status *s){
	uint8_t i,j;
	for(i = 0;i<N_SLOT;i++){
		for(j = 0;j<4;j++){
			s->Route_table[i].Dst[j] = 0;	 
			s->Route_table[i].Next_hop[j] = 0;
		}	
		s->Route_table[i].Metric = 255;
	}
	//itself
	s->Route_table[s->MAC-1].Dst[3] = s->MAC;	 
	s->Route_table[s->MAC-1].Next_hop[3] = s->MAC;
	s->Route_table[s->MAC-1].Metric = 0;
}

/*
*	delete id in the router table
*/
void Delete_router(Status *s , uint8_t id){
	uint8_t i;
	for(i=0;i<32;i++){				//delete all nodes who need id as next hop	 
		if(s->Route_table[i].Next_hop[3] == id){
			s->Route_table[i].Dst[3] = 0;	 
			s->Route_table[i].Next_hop[3] = 0;
			s->Route_table[i].Metric = 255;	
		}
	}
}

/*
*	get the information of its voisin and update its router table
*/
void Add_router(Status *s , uint8_t id, uint32_t voisin){	//after recieving the beacon , update the route table
	uint8_t i;
	uint32_t tmp;
	if(!Is_voisin(s,id)){					//only listen to voisin 
		Add_voisin(s,id);
		s->Route_table[id-1].Dst[3] = id;	 
		s->Route_table[id-1].Next_hop[3] = id;	 
		s->Route_table[id-1].Metric = 1;
	}
	for(i=0;i<32;i++){					 
		tmp = puissance(i);
		if((voisin&tmp)==tmp &&  !Is_voisin(s,i+1)){	//if it is not my voisin ; add it
			s->Route_table[i].Dst[3] = i+1;	 
			s->Route_table[i].Next_hop[3] = id;	 
			s->Route_table[i].Metric = 2;
		}
	}
}

/*
*	find th next hop for a certain dst
*/
uint8_t Find_next_hop(Status *s , uint8_t dst){			//find the next hop for routing
	if(s->Route_table[dst-1].Dst[3] == 0){			// it is not in route table
		return Default_GW(s);
	}else{
		return s->Route_table[dst-1].Next_hop[3];	//find the next hop
	}
}

/*
*	find the index which correspond certain id in the rip packet
*/
uint8_t Find_index(uint8_t id ,mPacket *m){			
	uint8_t i;
	for(i = 0;i<((m->length-10)/9);i++){
		if(m->payload.route[i].Dst[3] == id){
			return i;
		}
	}
	return 32;				//if failed, not exist
}

/*
*	with the rip recieved , then update the router table
*/
void Update_rip(Status *s ,mPacket *m){
	uint8_t i ,src, index;
	src = m->src[3];
	if(Is_voisin(s,src)){
		for(i = 0;i<32;i++){
			if(s->Route_table[i].Dst[3]==0 &&((index=Find_index(i+1,m))!=32 && s->Route_table[i].Next_hop[3] != s->MAC)){
				s->Route_table[i].Dst[3] = i+1;	 
				s->Route_table[i].Next_hop[3] = src; 
				s->Route_table[i].Metric = m->payload.route[index].Metric +1;
			}else if(s->Route_table[i].Dst[3]!=0 && (Find_index(i+1,m)==32) && s->Route_table[i].Next_hop[3] == src ){
				s->Route_table[i].Dst[3] = 0;	 
				s->Route_table[i].Next_hop[3] = 0; 
				s->Route_table[i].Metric = 255;
			}
		}
	}
}

/*
*	print the sensors who are on line , reachable
*/
void Show_Online(Status *s){
	uint8_t i;
	char on[3]="";
	print("\n\r");
	for(i=0;i<32;i++){					 
		if(s->Route_table[i].Dst[3]!=0 && (i!= (s->MAC-1))){ 
			print(" ");
			on[0] = (i+1)/10+'0';
			on[1] = (i+1)%10+'0';
			print(on);
			on[0] = 0;
			on[1] = 0;
		}
	}
	print("\n\rchoose one !\n\r");
}

/*
*	print the router table
*/
void Show_router(Status *s){
	uint8_t i;
	char ss[3]="";
	print("\n\r router table is as follows: \n\r");
	for(i=0;i<32;i++){					 
		if(s->Route_table[i].Dst[3]!=0){ 
			print("Dst:");
			ss[0] = (i+1)/10+'0';
			ss[1] = (i+1)%10+'0';
			print(ss);
			print(" Next_hop:");
			ss[0] = s->Route_table[i].Next_hop[3]/10 +'0';
			ss[1] = s->Route_table[i].Next_hop[3]%10 +'0';
			print(ss);
			print(" Metric:");
			ss[0] = s->Route_table[i].Metric/10 +'0';
			ss[1] = s->Route_table[i].Metric%10 +'0';
			print(ss);
			print("\n\r");
		}
	}
	print("That's all! \n\r");
}




