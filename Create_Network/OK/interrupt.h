#ifndef INTERRUPT_H 
#define INTERRUPT_H 

void Scan_Init(volatile Status * s);	
void Button_Init(void );
void Start_Timer(volatile Status* s);
void Stop_Timer(void );
void Start_Timer_Surveille(void );
void timer_send_beacon(volatile Status* s); 
void timer_synchrone(volatile Status* s);
void timer_message(volatile Status * s);			 
void timer_sleep(volatile Status * s);			 

#endif
