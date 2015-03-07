/*
 * =====================================================================================
 *
 *       Filename:  response.h
 *
 *    Description:  simple wrapper for http response.
 *
 *        Version:  1.0
 *        Created:  05/09/2012 01:58:39 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _RESPONSE_H
#define _RESPONSE_H

#include "header.h"
#include "request.h"
#include "server.h"
#include "evbuffer.h"

/** a simple status code collection */
enum status_code{
    RESPONSE_OK = 200,        /*!< ok */
    RESPONSE_BADREQ = 400,    /*!< bad request */
    RESPONSE_FORBIDDEN = 403, /*!< forbidden */
    RESPONSE_NOTFOUND = 404,  /*!< not found */
    RESPONSE_NOTIMP = 501,    /*!< not implemented */
};

/** message for status code */
#define RESPONSE_OK_MSG "OK"
#define RESPONSE_BADREQ_MSG "BAD REQUEST"
#define RESPONSE_FORBIDDEN_MSG "FORBIDDEN"
#define RESPONSE_NOTFOUND_MSG "NOT FOUND"
#define RESPONSE_NOTIMP_MSG "NOT IMPLEMENTED"

/** a useful macro to convert status code to status message */
#define CODE_2_MSG(code) code##_MSG

/** http response */
struct http_response{
    char *version;        /*!< http version HTTP/x.x */
    int status_code;      /*!< status code */
    char *status_msg;     /*!< status message */
    char *req_uri;      /*!< the request uri */
    char *decoded_uri;    /*!< decoded absolute uri path for request uri*/
    struct http_header_head *headers; /*!< response header */
    struct evbuffer *body_buf; /*!< response body */
    int type; /*!< response type ,0 for static ,1 for dynamic,this is used for internal */

    struct http_request *hrequest; /*!< the http request,this is used for internal */
    struct http_server *hserver; /*!< the http server,this is used for internal */
};

/**
  create a new reponse for a given http server to a http request

  @param hserver pointer to a http_server which the response is created for
  @param hrequest pointer to a http_request the server is response to
  @return a new http response,NULL if error occured
 */
struct http_response *hresponse_new(struct http_server *hserver,struct http_request *hrequest);

/**
  write the response to the given connection fd

  @param hresponse the http response to write
  @param connfd the connected fd
  @return 0 for successful,-1 if error occured
 */
int hresponse_write(struct http_response *hresponse,int connfd);

/**
  free the http response

  this function must be call at last to release all resources

  @param hresponse the http response to free
  @return 0 for successful,-1 if error occured
 */
int hresponse_free(struct http_response *hresponse);

#endif
