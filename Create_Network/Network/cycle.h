#ifndef CYCLE_H 
#define CYCLE_H 

void SendmPacket(mPacket *src ,mrfiPacket_t *dst);
void RecievemPacket(mrfiPacket_t *src ,mPacket *dst);
void Send_beacon(Status * s);
void Send_message(Status * s, char Mess[] , uint8_t  Destination);
void Sleep();

#endif
