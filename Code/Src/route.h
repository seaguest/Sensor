#ifndef ROUTE_H 
#define ROUTE_H 

#include "common.h"
#include <mrfi.h> 
#include "interrupt.h"
#include "uart.h" 
#include "cycle.h" 
#include "stdio.h" 
#include "stdlib.h"
#include "synchrone.h"
#include "fifo.h" 

uint32_t puissance(uint8_t i);
void Init_voisin(volatile Status *s);
void Add_voisin(volatile Status *s, uint8_t id);
void Delete_voisin(volatile Status *s, uint8_t id);
uint8_t Is_voisin(volatile Status *s, uint8_t dst);
uint8_t Default_GW(volatile Status *s);
void Init_route_table(volatile Status *s);
void Add_router(volatile Status *s , uint8_t id, uint32_t voisin);
void Delete_router(volatile Status *s , uint8_t id);
uint8_t Find_next_hop(volatile Status *s , uint8_t dst);
void Show_Online(volatile Status *s);
void Show_router(volatile Status *s);
uint8_t Find_index(uint8_t id ,mPacket *m);
void Update_rip(volatile Status *s ,mPacket *m);
void Show_voisin(volatile Status *s);
void Tidy_table(volatile Status *s);
void Set_filteredtable(volatile Status *s);

#endif
