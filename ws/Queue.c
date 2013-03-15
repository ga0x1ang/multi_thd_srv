#include "Queue.h"


/*
 * Initialize a Queue with head and tail pointers,
 * include the length information
 * qp is the pointer to the Queue pointer which points to the Queue
 */
void
InitQueue(Queue **qp)
{
    Queue *queue = malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->length = 0;
    *qp = (Queue *)&queue;
}

/*
 * Add an element into queue
 */
void
EnQueue(Queue **qp, int fd)
{
    Node *new_node = Node_create(fd);
    if((*qp)->length == 0)
    {
        (*qp)->head = new_node;
        (*qp)->tail = new_node;
    }
    else
    {
        (*qp)->tail->next = new_node;
        (*qp)->tail = new_node;
    }
    (*qp)->length++;
}

/*
 * Get and remove an element from a queue
 * TODO: no free() after remove from the queue
 */
int
DeQueue(Queue **qp)
{
    int fd;
    if((*qp)->head != NULL)
    {
        fd = (*qp)->head->data;

        if((*qp)->length == 1)
        {
            (*qp)->tail = NULL;
        }

        (*qp)->head = (*qp)->head->next;
        (*qp)->length--;
    }
    else
    {
        fd = -1;
    }
    return fd;
}
