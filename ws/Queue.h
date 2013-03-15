#ifndef QUEUE_H
#define QUEUE_H

#include "LinkedNode.h"

typedef struct Queue {
    Node *head;
    Node *tail;
    int length;
} Queue;

void InitQueue(Queue**);
void EnQueue(Queue**, int);
int DeQueue(Queue**);

#endif
