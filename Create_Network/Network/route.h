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
uint8_t Is_reachable(Status *s, uint8_t dst);
uint8_t Default_GW(Status *s);

#endif
