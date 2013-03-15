#include "LinkedNode.h"

/*
 * Alloc memory for a new Node and assigned it's pointers
 * Then return the pointer to this Node
 */
Node*
Node_create(int data)
{
    Node *new_node;

    new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = NULL;

    return new_node;
}
