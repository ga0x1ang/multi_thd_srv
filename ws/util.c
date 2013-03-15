/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <server.h>
#include <simple_http.h>
#include <content.h>

/* 
 * newfd_create_req and respond_and_free_req functions are there to
 * help you.  They are utility functions that hide some of the
 * complications of reading and writing to the file descriptors.
 * Hopefully you won't have to modify these two functions (or any of
 * the functions in simple_http, server, and content).
 */

/* 
 * Read data off of the new file descriptor (returned from
 * server_accept), and create a http_req object.  This struct
 * describes the request being made, including the path that is
 * requested (r->path).
 */
#define MAX_REQ_SZ 1024
struct http_req *
newfd_create_req(int new_fd)
{
	struct http_req *r;
	char *data;
	int amnt;

	data = malloc(MAX_REQ_SZ * sizeof(char));
	if (!data) return NULL;

	amnt = read(new_fd, data, MAX_REQ_SZ);
	if (amnt < 0) {
		//perror("read off of new file descriptor");
		free(data);
		return NULL;
	}

	r = shttp_alloc_req(new_fd, data);
	if (!r) {
		printf("Could not allocate request\n");
		free(data);
		return NULL;
	}
	if (shttp_get_path(r)) {
		data[amnt] = '\0';
		printf("Incorrectly formatted HTTP request:\n\t%s\n", data);
		shttp_free_req(r);
		return NULL;
	}
	return r;
}

/* 
 * Once a request has been formulated (of type char *, with length
 * len), then we want to write it out to the client through the file
 * descriptor.  This function will do that, and when it is done, it
 * will free the request structure, and all memory associated with it.
 */
void 
respond_and_free_req(struct http_req *r, char *response, int len)
{
	int amnt_written = 0;

	if (shttp_alloc_response_head(r, response, len)) {
		printf("Could not formulate HTTP response\n");
		shttp_free_req(r);
		return;
	}

	/* 
	 * At this point, we have the response, and the http head to
	 * reply with.  Write them out to the client!
	 */
	while (amnt_written != r->resp_hd_len) {
		int ret = write(r->fd, r->resp_head + amnt_written, 
				r->resp_hd_len - amnt_written);
		if (ret < 0) {
			printf("Could not write the response to the fd\n");
			goto done;
		}
		amnt_written += ret;
	}
	
	amnt_written = 0;
	while (amnt_written != r->resp_len) {
		int ret = write(r->fd, r->response + amnt_written, 
				r->resp_len - amnt_written);
		if (ret < 0) {
			printf("Could not write the response to the fd\n");
			goto done;
		}
		amnt_written += ret;
	}
done:
	shttp_free_req(r);
	return;
}

/* 
 * Process a client request on a newly opened file descriptor.
 */
void
client_process(int fd)
{
	struct http_req *r;
	char *response;
	int len;

	/* 
	 * This code will be used to get the request and respond to
	 * it.  This should probably be in the worker
	 * threads/processes.
	 */
	r = newfd_create_req(fd);
	if (!r || !r->path) {
		close(fd);
		return;
	}
	assert(r);
	assert(r->path);

	response = content_get(r->path, &len);
	if (!response) {
		shttp_free_req(r);
		return;
	}

	respond_and_free_req(r, response, len);
}
