/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */
/* 
 * This is a HTTP server.  It accepts connections on port 8080, and
 * serves a local static document.
 *
 * The clients you can use are 
 * - httperf (e.g., httperf --port=8080),
 * - wget (e.g. wget localhost:8080 /), 
 * - or even your browser.  
 *
 * To measure the efficiency and concurrency of your server, use
 * httperf and explore its options using the manual pages (man
 * httperf) to see the maximum number of connections per second you
 * can maintain over, for example, a 10 second period.
 *
 * Example usage:
 * # make test1
 * # make test2
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>

#include "util.h"		/* client_process */
#include "server.h"		/* server_accept and server_create */

#include "cas.h"

#include "thd_bounded.h"
#include "thd_lockfree.h"

#define MAX_DATA_SZ 1024
#define MAX_CONCURRENCY 4 /* Number of worker threads */
#define BUFFER_LENGTH 256 /* Maximum lengthe of the request queue*/


pthread_t master, worker[MAX_CONCURRENCY];
/* 
 * This is the function for handling a _single_ request.  Understand
 * what each of the steps in this function do, so that you can handle
 * _multiple_ requests.  Use this function as an _example_ of the
 * basic functionality.  As you increase the server in functionality,
 * you will want to probably keep all of the functions called in this
 * function, but define different code to use them.
 */
void
server_single_request(int accept_fd)
{
	int fd;

	/* 
	 * The server thread will always want to be doing the accept.
	 * That main thread will want to hand off the new fd to the
	 * new threads/processes/thread pool.
	 */
	fd = server_accept(accept_fd);
	client_process(fd);

    /* 
	 * A loop around these two lines will result in multiple
	 * documents being served.
	 */

	return;
}

/* 
 * The following implementations use a thread pool.  This collection
 * of threads is of maximum size MAX_CONCURRENCY, and is created by
 * pthread_create.  These threads retrieve data from a shared
 * data-structure with the main thread.  The synchronization around
 * this shared data-structure is either done using mutexes + condition
 * variables (for a bounded structure), or compare and swap (__cas in
 * cas.h) to do lock-free synchronization on a stack or ring buffer.
 */

void
server_thread_pool_bounded(int accept_fd)
{
    /* See thd_bounded.h */
    bounded_run(accept_fd, MAX_CONCURRENCY, BUFFER_LENGTH, &master, worker);
    return;
}

void
server_thread_pool_lock_free(int accept_fd)
{
    /* See thd_lockfree.h */
    lockfree_run(accept_fd, MAX_CONCURRENCY, BUFFER_LENGTH, &master, worker);
    return;
}

typedef enum {
	SERVER_TYPE_ONE = 0,
	SERVER_TYPE_THREAD_POOL_BOUND,
	SERVER_TYPE_THREAD_POOL_ATOMIC,
} server_type_t;

int
main(int argc, char *argv[])
{
	server_type_t server_type;
	short int port;
	int accept_fd;

	if (argc != 3) {
		printf("Proper usage of http server is:\n%s <port> <#>\n"
		       "port is the port to serve on, # is either\n"
		       "0: server only a single request\n"
		       "1: use a thread pool and a _bounded_ buffer with "
		       "mutexes + condition variables\n"
		       "2: use a thread pool and compare and swap to "
		       "implement a lock-free stack of requests\n",
		       argv[0]);
		return -1;
	}

	port = atoi(argv[1]);
	accept_fd = server_create(port);

    if (accept_fd < 0) return -1;
	
	server_type = atoi(argv[2]);

	switch(server_type) {
	case SERVER_TYPE_ONE:
		server_single_request(accept_fd);
		break;
	case SERVER_TYPE_THREAD_POOL_BOUND:
		server_thread_pool_bounded(accept_fd);
		break;
	case SERVER_TYPE_THREAD_POOL_ATOMIC:
		server_thread_pool_lock_free(accept_fd);
		break;
	}
	close(accept_fd);

	return 0;
}
