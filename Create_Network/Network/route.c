#include"route.h"

uint32_t puissance(uint8_t i){
	uint32_t tmp= 1;
	while(i--){
		tmp *= 2;
	}
	return tmp;
}  

void Init_voisin(Status *s){
	s->Voisin = 0;	
	s->Voisin |= (uint32_t ) puissance(s->MAC);
}

void Add_voisin(Status *s, uint8_t id){
	s->Voisin |= (uint32_t ) puissance(id);
}

uint8_t Is_reachable(Status *s, uint8_t dst){		//find if the destination is in the table of router
	uint32_t  tmp;
	tmp = puissance(dst);
	if((s->Voisin&tmp)==tmp){
		return 1;
	}else{
		return 0;
	}
}

uint8_t Default_GW(Status *s){				//return the default gateway , we choose the first one
	uint32_t  i ,tmp;
	for(i=0;i<32;i++){
		tmp = puissance(i);
		if((s->Voisin&tmp)==tmp){
			return i;
		}
	}
	return BROADCAST;
}

