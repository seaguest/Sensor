#ifndef INTERRUPT_H 
#define INTERRUPT_H 

void Scan_Init(Status * s);				//open the timer of scan ; after the time over ; if not existe a network ;then create one
void timer_message(Status * s);				//after sending beacon , wait for sending message
void timer_sleep(Status * s);				//after sending message , wait for sleeping

void timer_synchrone(Status* s);
void timer_send_beacon(Status* s);

void Button_Init();
void Start_Timer(Status* s);
void Stop_Timer();

void Start_Timer_Surveille();

#endif
