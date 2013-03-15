#include "Stack.h"
#include "LinkedNode.h"

/* Initialize the Stack */
void
Stack_init(Stack **s)
{
    *s = (Stack*)malloc(sizeof(Stack));
    (*s)->top = NULL;
    (*s)->length = 0;
}

/* Push an element to the stack */
void
Stack_push(Stack **s, int data)
{
    Node *new_node = Node_create(data);

    if((*s)->top != NULL)
    {   
        new_node->next = (*s)->top;
        return;
    }
    (*s)->top = new_node;
    (*s)->length++;

    return;
}

/* Pop and element from the stack*/
int
Stack_pop(Stack **s)
{
    int ret;
    if((*s)->top != NULL)
    {
        ret = (*s)->top->data;
        (*s)->top = (*s)->top->next;
        (*s)->length--;
    }
    else
    {
        ret = -1;
    }
    return ret;
}
