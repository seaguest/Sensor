#include "fifo.h" 

int InitQueue(QList *Q)
{
	Q->front = Q->rear = (QNode *)malloc(sizeof(QNode));  
	if(!Q->front)
		return 0;
	Q->rear->next = NULL;             
	return 1;
}

int IsEmpty(QList *Q)
{
	if(Q->front == Q->rear){
		return 1;
	}else{
		return 0;
	}
}

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

QElemtype DeQueue(QList *Q)		//get the head
{
	QElemtype e;
	if(!IsEmpty(Q)){
		e = Q->front->next->data;
		Q->front = Q->front->next;
		return e;
	}
	return 0;
}

/*
int main()
{
	QList Q;
	InitQueue(&Q);
	char c,x = 'z';
	while(1){

		printf("\ninput fifo!\n");
		while ((c = getchar()) != '\n')
		{
			EnQueue(&Q,c);
		}

		printf("enqueue ok!\n");
		printf("\n size is %d \n", Length(&Q));


		if(Search(&Q, ' ')){
			printf("YES find it!\n");
			while (!IsEmpty(&Q))	//evry time one message
			{
				c = DeQueue(&Q);
				if( c == ' ' ){
					break;
				}
			}
		}else{
			printf("NO!\n");
		}
	}
	return 0;
}
*/

