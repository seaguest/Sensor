#ifndef UART_H 
#define UART_H 

void Uart_Init(void );				//initialisation de uart
void TXString(char* string, int length);	//send message in uart
void print(char *s);

#endif
