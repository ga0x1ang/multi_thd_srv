/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <simple_http.h>

struct http_req *
shttp_alloc_req(int fd, char *request)
{
	struct http_req *r;
	assert(request);

	r = malloc(sizeof(struct http_req));
	if (!r) return NULL;
	memset(r, 0, sizeof(struct http_req));

	r->request = request;
	r->req_len = strlen(request);
	r->path = NULL;
	r->fd = fd;

	return r;
}

void 
shttp_free_req(struct http_req *r)
{
	r->path = NULL;
	if (r->request)   free(r->request);
	if (r->response)  free(r->response);
	if (r->resp_head) free(r->resp_head);
	close(r->fd);
	free(r);
}

static inline char *
find_whitespace(char *s) 
{ 
	assert(s);
	while (*s != ' ' && *s != '\0') s++;
	return s;
}

/* 
 * Pass in the request.  Set the ->path field in r to point to the
 * path that is being requested.
 */
int 
shttp_get_path(struct http_req *r)
{
	char *curr, *end, *path;

	assert(r);
	assert(r->request);
	
	if (strncmp(r->request, "GET ", strlen("GET "))) return -1;
	path = curr = r->request + sizeof("GET ")-1;

	end = find_whitespace(curr);
	if (*end == '\0') return -1;
	*end = '\0';

	if (*path == '/') path++;
	r->path = path;
	
	return 0;
}

static const char success_head[] =
	"HTTP/1.1 200 OK\r\n"
	"Date: Sat, 14 Feb 2008 14:59:00 GMT\r\n"
	"Connection: close\r\n"
	"Content-Length: ";
#define MAX_DIGITS 128

/* 
 * Creates the ->response field in r that includes "answer", but also
 * includes other data.
 */ 
int 
shttp_alloc_response_head(struct http_req *r, char *data, int dlen)
{
	int  head_sz, pre_sz, len_sz;
	char len_str[MAX_DIGITS];
	char *resp_hd;

	r->response = data;
	r->resp_len = dlen;

	len_sz  = snprintf(len_str, MAX_DIGITS, "%d\r\n\r\n", r->resp_len);
	if (len_sz < 1) return -1;
	
	pre_sz  = strlen(success_head);
	assert(pre_sz > 0);
	head_sz = pre_sz + len_sz;
	assert(head_sz > 0);
	resp_hd = malloc(head_sz);
	if (!resp_hd)   return -1;

	r->resp_head   = resp_hd;
	r->resp_hd_len = head_sz;
	
	memcpy(resp_hd, success_head, pre_sz);
	memcpy(resp_hd + pre_sz, len_str, len_sz);

	return 0;
}
