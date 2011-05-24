#ifndef INTERRUPT_H 
#define INTERRUPT_H 

void wait_beacon_first(Status * s);	//synchronisation for the first time with the beacon received
void Scan_Init(Status * s);				//open the timer of scan ; after the time over ; if not existe a network ;then create one
void timer_wait_message(Status * s);				//after sending beacon , wait for sending message
void timer_wait_sleep(Status * s);				//after sending message , wait for sleeping
void timer_wait_beacon(Status * s);				//after sleeping , wait for beacon
void timer_host_wait_beacon(Status * s);
void timer_host_wait_message(Status * s);

void Timer_synchrone();
void Button_Init();
void Start_Timer();
void Stop_Timer();

#endif
