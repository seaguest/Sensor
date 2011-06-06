#include "fifo.h" 
#include "string.h" 

/*
*	we use a FIFO for sending and recieving the message 
*	here are the functions of manipuling the FIFO
*/


/*
*	initialisation for a FIFO
*/
int InitQueue(QList *Q)
{
	Q->front = (QNode *)malloc(sizeof(QNode));  	//apply for the memory
	Q->rear = Q->front ;
	if(!Q->front)
		return 0;	
	Q->front->next = NULL;
	Q->rear->next = NULL;
	return 1;
}

/*
*	clean the FIFO
*/
void CleanQueue(QList *Q)
{
	QLink p = Q->front;
	while(p != NULL){
		Q->front = Q->front->next;
		free(p);
		p = Q->front;
	}  
	Q->rear = NULL;       
}


/*
*	check if the FIFO is empty
*/
int IsEmpty(QList *Q)
{
	if(Q){					// if Q is not null
		if(Q->front == Q->rear){
			return 1;
		}else{
			return 0;
		}
	}
	return 0;
}

/*
*	calcule the lenth of the FIFO
*/
int Length(QList *Q)
{
	int l = 0;
	QLink p = Q->front;
	while(p != Q->rear){
		p = p->next;	
		l++;	
	}
	return l;
}

/*
*	search an element in the FIFO
*/
int Search(QList *Q ,QElemtype e)
{
	QLink p = Q->front;
	while(p != Q->rear){
		if(p->next->data == e){
			return 1;
		}else{
			p = p->next;	
		}
	}
	return 0;
}

/*
*	enqueue an element in the FIFO
*/
int EnQueue(QList *Q,QElemtype e)
{
	QLink p = (QNode *)malloc(sizeof(QNode));   
	if(!p)
		return 0;
	p->data = e;                  
	p->next = NULL;
	Q->rear->next = p;                         
	Q->rear = p;
	return 1;
}

/*
*	dequeue an element from the FIFO
*/
QElemtype DeQueue(QList *Q)	
{
	QElemtype e;
	QNode *p;
	if(!IsEmpty(Q)){
		p = Q->front->next;
		e = p->data;
		if(Q->rear == Q->front->next){
			Q->rear = Q->front;
			Q->rear->next = NULL;
			Q->front->next = NULL;
		}else{
			Q->front->next = Q->front->next->next;
		}
		free(p);
		return e;
	}
	return 0;
}

