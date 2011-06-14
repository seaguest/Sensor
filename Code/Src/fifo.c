
/***************************************************************************************
Copyright (C), 2011-2012, ENSIMAG.TELECOM 
File name	: fifo.c
Author		: HUANG yongkan & KANJ mahamad
Version		:
Date		: 2011-6-6
Description	: we use a FIFO for sending and recieving the message 
		  here are the functions of manipuling the FIFO
Function List	:  
		  void InitQueue(volatile QList *Q);			// initialisation for a FIFO
		  uint8_t IsEmpty(volatile QList *Q);			// check if the buffer is empty	
		  uint8_t IsFull(volatile QList *Q);			// check if the buffer is full
		  uint8_t Length(volatile QList *Q);			// calcule the lenth of the buffer
		  uint8_t Search(volatile QList *Q ,QElemtype e);	// search an element in the buffer
		  uint8_t EnQueue(volatile QList *Q,QElemtype e);	// enqueue an element in the buffer
		  QElemtype DeQueue(volatile QList *Q);			// dequeue an element from the buffer
 		  //void print_fifo(volatile QList *Q);			// this function is used for debugging the fifo
***************************************************************************************/

#include "fifo.h" 
#include "string.h" 
#include "uart.h" 


/***************************************************************************************
Function	: void InitQueue(volatile QList *Q)
Description	: initialisation for a FIFO
Calls		:  
Called By	: void Synchrone_Init(uint8_t mac)
		  interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: volatile QList *Q
Output		: the FIFO is reset
Return		: void
Others		: 
***************************************************************************************/

void InitQueue(volatile QList *Q)
{
	Q->front = Q->rear = 0;
}


/***************************************************************************************
Function	: uint8_t IsEmpty(volatile QList *Q)
Description	: check if the buffer is empty
Calls		:  
Called By	: void Recieve_message(volatile Status * s, volatile QList *p)
		  void Send_message(volatile Status * s, volatile QList *Q, uint8_t  dst)
		  QElemtype DeQueue(volatile QList *Q)	
		  void print_fifo(volatile QList *Q)
		  uint8_t Search(volatile QList *Q ,QElemtype e)
Input		: volatile QList *Q
Output		: 
Return		: 1	fifo is empty
		  0 	fifo is not empty
Others		: 
***************************************************************************************/

uint8_t IsEmpty(volatile QList *Q)
{
	return (Q->rear == Q->front);
}


/***************************************************************************************
Function	: uint8_t IsFull(volatile QList *Q)
Description	: check if the buffer is full
Calls		:  
Called By	: uint8_t EnQueue(volatile QList *Q,QElemtype e)
Input		: volatile QList *Q
Output		: 
Return		: 1	fifo is full
		  0 	fifo is not full
Others		: 
***************************************************************************************/

uint8_t IsFull(volatile QList *Q)
{
	return ((Q->rear+1)%MAXSIZE == Q->front);
}

/***************************************************************************************
Function	: uint8_t Length(volatile QList *Q)
Description	: calcule the lenth of the buffer
Calls		:  
Called By	: void Send_message(volatile Status * s, volatile QList *Q, uint8_t  dst)
		  void print_fifo(volatile QList *Q)
Input		: volatile QList *Q
Output		: 
Return		: the length of FIFO
Others		: 
***************************************************************************************/

uint8_t Length(volatile QList *Q)
{
	return (Q->rear - Q->front + MAXSIZE)%MAXSIZE;	
}


/***************************************************************************************
Function	: uint8_t Search(volatile QList *Q ,QElemtype e)
Description	: search an element in the buffer
Calls		:  
Called By	: void Send_message(volatile Status * s, volatile QList *Q, uint8_t  dst)
		  void Recieve_message(volatile Status * s, volatile QList *p)
Input		: volatile QList *Q ,QElemtype e
Output		: 
Return		: 1	if find element e in fifo
  		  0	if not find element e in fifo
Others		: 
***************************************************************************************/

uint8_t Search(volatile QList *Q ,QElemtype e)
{
	uint8_t i, end;
	if(!IsEmpty(Q)){
		end = Q->rear > Q->front ? Q->rear : (Q->rear + MAXSIZE);
		for(i = Q->front; i< end; i++){
			if(Q->buffer[i%MAXSIZE] == e){
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

/***************************************************************************************
Function	: uint8_t EnQueue(volatile QList *Q,QElemtype e)
Description	: enqueue an element in the buffer
Calls		:  
Called By	: void MRFI_RxCompleteISR()
		  interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: volatile QList *Q,QElemtype e
Output		: 
Return		: 1	if enqueue successfully
  		  0	if enqueue failed
Others		: 
***************************************************************************************/

uint8_t EnQueue(volatile QList *Q,QElemtype e)
{
	if(!IsFull(Q)){
		Q->buffer[Q->rear] = e;
		Q->rear = (Q->rear+1)%MAXSIZE; 
		return 1;
		return 0;
	}
	return 0;
}

/***************************************************************************************
Function	: QElemtype DeQueue(volatile QList *Q)	
Description	: dequeue an element from the buffer
Calls		:  
Called By	: void Recieve_message(volatile Status * s, volatile QList *p)
		  void Send_message(volatile Status * s, volatile QList *Q, uint8_t  dst)
Input		: volatile QList *Q
Output		: 
Return		: 1	if dequeue successfully
  		  0	if dequeue failed
Others		: 
***************************************************************************************/

QElemtype DeQueue(volatile QList *Q)	
{
	QElemtype e;
	if(!IsEmpty(Q)){
		e = Q->buffer[Q->front];
		Q->front = (Q->front+1)%MAXSIZE; 
		return e;
	}
	return 0;
}

/***************************************************************************************
Function	: void print_fifo(volatile QList *Q)
Description	: this function is used for debugging the fifo
Calls		:  
Called By	: interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
Input		: volatile QList *Q
Output		: 
Return		: void
Others		: 
***************************************************************************************/

/*
void print_fifo(volatile QList *Q)
{
	uint8_t i, end;
	char o[20] = "";
	if(!IsEmpty(Q)){
		print("fifo: \n\r");
		print_8b(Q->front);
		print_8b(Q->rear);
		print_8b(Length(Q));
		TXString("\n\r",2);

		end = Q->rear > Q->front ? Q->rear : (Q->rear + MAXSIZE);
		for(i = Q->front; i< end; i++){
			if( Q->buffer[i%MAXSIZE] == '\r'){
				o[i - Q->front] = 'x';
			}else{
				o[i - Q->front] = Q->buffer[i%MAXSIZE];
			}
		}
		TXString(o,Length(Q));
		print("\n\rover \n\r");
	}
}
*/

