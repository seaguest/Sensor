#ifndef ROUTE_H 
#define ROUTE_H 

#include "common.h"
#include <mrfi.h> 
#include <string.h> 
#include "interrupt.h"
#include "uart.h" 
#include "cycle.h" 
#include "stdio.h" 
#include "stdlib.h"
#include "synchrone.h"
#include "fifo.h" 

uint32_t puissance(uint8_t i);
void Init_voisin(Status *s);
void Add_voisin(Status *s, uint8_t id);
uint8_t Is_voisin(Status *s, uint8_t dst);
uint8_t Default_GW(Status *s);

void Init_route_table(Status *s);
void Add_router(Status *s , uint8_t id, uint32_t voisin);
uint8_t Find_next_hop(Status *s , uint8_t dst);

void Delete_router(Status *s , uint8_t id);
void Delete_voisin(Status *s, uint8_t id);
void Show_Online(Status *s);
void Show_router(Status *s);
uint8_t Find_index(uint8_t id ,mPacket *m);

#endif
