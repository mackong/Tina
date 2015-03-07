/*
 * =====================================================================================
 *
 *       Filename:  header.h
 *
 *    Description:  simple wrapper for http request/response header
 *
 *        Version:  1.0
 *        Created:  05/02/2012 09:34:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _HEADER_H
#define _HEADER_H

#include <sys/queue.h> /* for TAIL QUEUE */

/** http header */
struct http_header{
    char* name; /*!< header name */
    char* value; /*!< header value */
    TAILQ_ENTRY(http_header) next; /*!< queue */
};

/** 
   http_header queue head definition.
 */
TAILQ_HEAD(http_header_head,http_header);

/**
  create a new http_header queue.

  @return a pointer to http_header_head,NULL if error occured
 */
struct http_header_head *hheader_new();

/**
  free a http_header queue.

  @param hqueue pointer to a http_header_head to free
 */
void hheader_free(struct http_header_head *hqueue);

/**
  add a http header to the header queue.

  @param hqueue pointer to a http_header_head to add
  @param line the (name:value) line to add
  @return 0 for success,-1 if error occured
 */
int hheader_add_line(struct http_header_head *hqueue, char *line);

/**
  add a name-value pair to the header queue.

  @param hqueue pointer to a http_header_head to add
  @param name the name to add
  @param value the vlaue to add
  @return 0 for success,-1 if error occured
 */
int hheader_add_pair(struct http_header_head *hqueue, char *name,char *value);


/**
  get a header value of a given header name.

  @param hqueue pointer to a http_header_head to get value from
  @param name the header name
  @return if found,return pointer to the header value,otherwise return null
 */
const char *hheader_get_value(struct http_header_head *hqueue,const char *name);

#endif
