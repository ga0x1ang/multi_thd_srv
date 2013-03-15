#include <pthread.h>

#include "server.h"
#include "util.h"
#include "Queue.h"

pthread_mutex_t bounded_mutex;
pthread_cond_t bounded_cond;
Queue *newQueue;

/* The master thread which is responsibile for accepting request
 * and add it to the queue
 * args[] is an integer array, includes:
 * 1. server socket fd
 * 2. maximum queue size(BUFFER_LENGTH)
 */
void*
bounded_master_routine(int args[])
{
    int fd;

    int accept_fd = args[0];
    int max_queue_size = args[1];

    while(1)
    {
        fd = server_accept(accept_fd);
        if(fd > 0)
        {
            pthread_mutex_lock(&bounded_mutex);
            while(newQueue->length == max_queue_size) /* wait when the queue is full */
            {
                printf("queue is full\n");
                pthread_cond_wait(&bounded_cond, &bounded_mutex);
            }

            /* The actural processing part,
             * create a new node for this request and append it to the queue
             * ----BEGIN----
             */
            EnQueue(&newQueue, fd);
            /* -----END----*/
            
            pthread_cond_signal(&bounded_cond);
            pthread_mutex_unlock(&bounded_mutex);
        }
    }
    return NULL;
}

/* The worker thread routine
 * this routine will get request from queue and process it
 */
void*
bounded_worker_routine()
{
    int fd;

    while(1)
    {
        pthread_mutex_lock(&bounded_mutex);
        while(newQueue->length == 0) /* wait when queue is empty */
        {
            pthread_cond_wait(&bounded_cond, &bounded_mutex);
        }

        /* The actural processing part,
         * get the data(int) from the last node of the queue and remove the node
         * ----BEGIN----
         */
        fd = DeQueue(&newQueue);
        client_process(fd);
        /* -----END----*/

        pthread_cond_signal(&bounded_cond);
        pthread_mutex_unlock(&bounded_mutex);
    }

    return NULL;
}


/*
 * The entry of this threading method, take in all the environment variables and initialize all the threads
 */
void
bounded_run(int accept_socket, int concurrency, int max_queue_size, pthread_t *master, pthread_t *worker)
{
    /* Initialize the variables */
    pthread_mutex_init(&bounded_mutex, NULL);
    pthread_cond_init(&bounded_cond, NULL);
    newQueue = NULL;
    InitQueue(&newQueue);
    int args[] = {accept_socket, max_queue_size};

    /* Create the master thread for listening the socket and add received request to the queue */
    pthread_create(master, NULL, (void*)bounded_master_routine, (void*)&args);
    
    /* Create the worker thread for listening the socket and add received request to the queue */
    int i;
    for(i = 0; i < concurrency; i++)
    {
        pthread_create(&worker[i], NULL, (void*)bounded_worker_routine, NULL);
    }

    while(1)
        ;


    /* the cleaning work */
    pthread_join(*master, NULL);
    for(i = 0; i<concurrency; i++)
    {
        pthread_join(worker[i], NULL);
    }

    pthread_mutex_destroy(&bounded_mutex);
    pthread_cond_destroy(&bounded_cond);
    free(&args);
}
