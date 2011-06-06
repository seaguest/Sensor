#ifndef CYCLE_H 
#define CYCLE_H 

#include "fifo.h" 

void SendmPacket(mPacket *src ,mrfiPacket_t *dst);
void RecievemPacket(mrfiPacket_t *src ,mPacket *dst);
void Send_beacon(Status * s);
void Send_rip(Status * s);
void Send_message(Status * s, QList *Q, uint8_t  Destination);
void Recieve_message(Status * s, QList *Q);
void Sleep(void);

#endif
