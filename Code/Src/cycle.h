#ifndef CYCLE_H 
#define CYCLE_H 

#include "fifo.h" 

void SendmPacket(mPacket *src ,mrfiPacket_t *dst);
void RecievemPacket(mrfiPacket_t *src ,mPacket *dst);
void Send_beacon(volatile Status * s);
void Send_rip(volatile Status * s);
void Send_message(volatile Status * s, volatile QList *Q, uint8_t  Destination);
void Recieve_message(volatile Status * s, volatile QList *Q);
void Sleep(void);

#endif
