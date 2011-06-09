#ifndef INTERRUPT_H 
#define INTERRUPT_H 

void Scan_Init(volatile Status * s);				//open the timer of scan ; after the time over ; if not existe a network ;then create one
void timer_message(volatile Status * s);				//after sending beacon , wait for sending message
void timer_sleep(volatile Status * s);				//after sending message , wait for sleeping

void timer_synchrone(volatile Status* s);
void timer_send_beacon(volatile Status* s);

void Button_Init(void );
void Start_Timer(volatile Status* s);
void Stop_Timer(void );

void Start_Timer_Surveille(void );
uint8_t Clock(void );
void Set_Clock(void );

#endif
