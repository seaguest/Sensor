#ifndef INTERRUPT_H 
#define INTERRUPT_H 

void wait_beacon_first(uint8_t ID_Beacon);	//synchronisation for the first time with the beacon received
void Scan_Init(void);				//open the timer of scan ; after the time over ; if not existe a network ;then create one
void wait_message();				//after sending beacon , wait for sending message
void wait_sleep();				//after sending message , wait for sleeping
void wait_beacon();				//after sleeping , wait for beacon

void Button_Init();
void Timer_Init();

#endif
