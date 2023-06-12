#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__


typedef struct node{
	char buffer[16];
    int client_socket;
	struct node *link;
}Q_node; 

typedef struct queue{
	int    size;
	Q_node *head, *tail;
}Queue;

void initQueue(Queue *q);

int queueIsEmpty(Queue *q);

void addQueue(Queue *q, char* msg);

Q_node* deleteQueue(Queue *q);

#endif