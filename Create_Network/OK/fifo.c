#include "fifo.h" 
#include "string.h" 
#include "uart.h" 


/*
*	we use a FIFO for sending and recieving the message 
*	here are the functions of manipuling the FIFO
*/

 
/*
*	initialisation for a FIFO
*/
void InitQueue(volatile QList *Q){
	Q->front = Q->rear = 0;
}

/*
*	check if the buffer is empty
*/
uint8_t IsEmpty(volatile QList *Q){
	return (Q->rear == Q->front);
}

/*
*	check if the buffer is full
*/
uint8_t IsFull(volatile QList *Q){
	return ((Q->rear+1)%MAXSIZE == Q->front);
}

/*
*	calcule the lenth of the buffer
*/
uint8_t Length(volatile QList *Q)
{
	return (Q->rear - Q->front + MAXSIZE)%MAXSIZE;	
}

/*
*	search an element in the buffer
*/
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

/*
*	enqueue an element in the buffer
*/
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

/*
*	dequeue an element from the buffer
*/
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

/*
*	this function is used for debugging the fifo
*/
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

