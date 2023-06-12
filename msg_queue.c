#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char buffer[16];
    struct node *link;
} Q_node;

typedef struct queue {
    int size;
    Q_node *head, *tail;
} Queue;

void initQueue(Queue *q) {
    q->size = 0;
    q->head = q->tail = NULL;
}

int queueIsEmpty(Queue *q) {
    return q->head == NULL;
}

void addQueue(Queue *q, char* msg) {
    Q_node *temp = (Q_node *) malloc(sizeof(Q_node));
    strcpy(temp->buffer, msg);
    temp->link = NULL;
    if (q->head == NULL) {
        q->head = temp;
    } else {
        q->tail->link = temp;
    }
    q->tail = temp;
    q->size++;
}

Q_node* deleteQueue(Queue *q) {
    Q_node *temp = q->head;
    Q_node*  retval = (Q_node *) malloc(sizeof(Q_node));
    strcpy(retval->buffer, temp->buffer);
    q->head = (q->head)->link;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    q->size--;
    free(temp);
    return retval;
}
