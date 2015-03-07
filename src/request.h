/*
 * =====================================================================================
 *
 *       Filename:  request.h
 *
 *    Description:  simple wrapper for http request
 *
 *        Version:  1.0
 *        Created:  05/02/2012 08:43:44 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _REQUEST_H
#define _REQUEST_H

#include "evbuffer.h"
#include "header.h"
#include "server.h"

/** request method */
enum req_method{
    REQ_GET, /*!< GET method */
    REQ_HEAD, /*!< HEAD method */
    REQ_POST, /*!< POST method  */
    REQ_UNKNOWN, /*!< server does not support */
};

/** http request */
struct http_request{
    enum req_method method; /*!< req_method */
    char *uri; /*!< resource uri */
    char *version; /*!< http version HTTP/x.x */
    struct http_header_head *headers; /*!< http headers queue */
    
    int is_cgi;   /*!< is cgi request or not */
    char *cgi_script; /*!< cgi script */
    char *cgi_query_string; /*!< cgi query string */
};

/**
  create a new http_request from evbuffer which is get from client.

  @param buf pointer to the evbuffer to create http_request
  @return pointer to a new http_request,null if error occured
 */
struct http_request *hrequest_new(struct http_server *hserver,struct evbuffer *buf);

/**
  free a http_request and free the resource.

  @param req pointer to http_request to free
 */
void hrequest_free(struct http_request *req);

/**
  process http request ,and give reponse corrosponding.

  @param req pointer to http_request to process of
  @return 0 for process successful,-1 if error occured
 */
int hrequest_process(struct http_request *req);

#endif
