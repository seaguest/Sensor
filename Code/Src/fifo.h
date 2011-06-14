#ifndef FIFO_H 
#define FIFO_H 

#include <stdio.h>
#include <stdlib.h>

#define MAXSIZE 100

typedef char QElemtype;

typedef struct{
	QElemtype buffer[MAXSIZE];
	uint8_t front;
	uint8_t rear;
} QList;

void InitQueue(volatile QList *Q);
uint8_t IsEmpty(volatile QList *Q);
uint8_t IsFull(volatile QList *Q);
uint8_t Length(volatile QList *Q);
uint8_t Search(volatile QList *Q ,QElemtype e);
uint8_t EnQueue(volatile QList *Q,QElemtype e);
QElemtype DeQueue(volatile QList *Q);

//void print_fifo(volatile QList *Q);

#endif
