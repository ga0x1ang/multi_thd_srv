#include <stdio.h>
#include <pthread.h>

#include "server.h"
#include "cas.h"
#include "util.h"
#include "Stack.h"

volatile Stack *lockfree_queue; /* This need to be volatile but will cause complier warnings */

/* The master thread routine which accept request and push it into stack
 * The arguments include the server socket(args[0]) and max_queue_size(args[1])
 */
void*
lockfree_master_routine(int args[])
{
    Stack *old_top = NULL;
    Stack *new_top = NULL;
    int fd;

    int lockfree_accept_fd = args[0];
    int lockfree_max_queue = args[1];

    while(1)
    {
        if(lockfree_queue->length == lockfree_max_queue)
            continue;

        fd = server_accept(lockfree_accept_fd);
        if(fd == -1) /* May be a invalid request*/
            continue;

        do {
            old_top = lockfree_queue;
            new_top = lockfree_queue;
            Stack_push(&new_top, fd);
        } while(__cas((unsigned long*)&lockfree_queue, (unsigned long)old_top, (unsigned long)new_top));
    }
    return NULL;
}

/* The worker thread routine which get request and process it */
void*
lockfree_worker_routine()
{
    Stack *old_top, *new_top;
    int fd;
    while(1)
    {
        if(lockfree_queue->length == 0)
            continue;
        do {
            old_top = lockfree_queue;
            new_top = lockfree_queue;
            fd = Stack_pop(&new_top);
            if(fd > 0)
                client_process(fd);
        } while(__cas((unsigned long*)&lockfree_queue, (unsigned long)old_top, (unsigned long)new_top));
    }
    return NULL;
}

/* The entry function which do all the initialization work and control the whole life cycle */
void
lockfree_run(int accept_socket, int concurrency, int max_queue_size, pthread_t *master, pthread_t *worker)
{
    /* Initialize the variables */
    Stack_init(&lockfree_queue);
    int args[] = {accept_socket, max_queue_size};

    /* Create the master thread */
    pthread_create(master, NULL, (void*)lockfree_master_routine, (void*)&args);

    /* Create the worker threads */
    int i;
    for(i = 0; i < concurrency; i++)
    {   
        pthread_create(&worker[i], NULL, (void*)lockfree_worker_routine, NULL);
    }   

    while(1)
        ;   

    /* The clean work */
    pthread_join(*master, NULL);
    for(i = 0; i<concurrency; i++)
    {   
        pthread_join(worker[i], NULL);
    }
    free(lockfree_queue);

}
