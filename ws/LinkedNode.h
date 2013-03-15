#ifndef LINKEDNODE_H
#define LINKEDNODE_H

#include <stdlib.h>

/* _________________________
 * |data | ptr to next node*|
 * -------------------------
 */
typedef struct Node {
    int data;
    struct Node *next;
} Node;

Node *Node_create(int);

#endif
