#ifndef FIFO_H 
#define FIFO_H 

#include <stdio.h>
#include <stdlib.h>

typedef char QElemtype;

typedef struct QNode{
	QElemtype data;
	struct QNode* next;
}QNode,*QLink;

typedef struct{
	QLink front;
	QLink rear;
}QList;

int InitQueue(QList *Q);
int DestoryQueue(QList *Q);
int EnQueue(QList *Q,QElemtype e);
QElemtype DeQueue(QList *Q);
int IsEmpty(QList *Q);
int Length(QList *Q);
int Search(QList *Q ,QElemtype e);

QList FIFO_Send,FIFO_Recieve;		//fifo for storing message to send

#endif
