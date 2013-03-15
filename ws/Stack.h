#ifndef STACK_H
#define STACK_H

#include "LinkedNode.h"

typedef struct Stack {
    Node *top;
    int length;
} Stack;

void Stack_init(Stack **s);
void Stack_push(Stack **s, int data);
int Stack_pop(Stack **s);

#endif
