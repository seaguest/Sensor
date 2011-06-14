#ifndef UART_H 
#define UART_H 

void Uart_Init(void );				 
void TXString(char* string, int length);	 
void print(char *s);
void print_8b(uint8_t u );
void print_16b(uint16_t u );
void print_32b(uint32_t u );

#endif
