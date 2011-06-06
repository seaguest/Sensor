#ifndef SYNCHRONE_H 
#define SYNCHRONE_H 
 
void Delay_Rand(uint32_t mod);
void Synchrone_Init(uint8_t mac);
void print(char *s);

void delay_reste_slot(void);
void Clear_Synchrone(void );
void Set_Synchrone(void );
void mutex(uint8_t *u ,uint8_t v );

#endif
