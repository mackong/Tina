/*
 * =====================================================================================
 *
 *       Filename:  request.c
 *
 *    Description:  simple wrapper for http request.
 *
 *        Version:  1.0
 *        Created:  05/02/2012 08:45:45 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "util.h"
#include "header.h"
#include "request.h"

/**
  parse initial line for a request.

  @param request result request
  @param line initial line to parse
  @return 0 for successful,-1 if error occured
 */
static int parse_initline(struct http_request *request,char *line)
{
    char *token = NULL;

    if(request == NULL || line == NULL){
        return -1;
    }

    /* method */
    token = strtok(line," ");
    if(strcmp(token,"GET") == 0){
        request->method = REQ_GET;
    }
    else if(strcmp(token,"HEAD") == 0){
        request->method = REQ_HEAD;
    }
    else if(strcmp(token,"POST") == 0){
        request->method = REQ_POST;
    }
    else{
        request->method = REQ_UNKNOWN;
    }

    /* uri */
    token = strtok(NULL," ");
    request->uri = strdup(token);

    /* http version */
    token = strtok(NULL," ");
    request->version = strdup(token);

    return 0;
}

struct http_request *hrequest_new(struct http_server *hserver,struct evbuffer *buf)
{
    char *line = NULL;
    int ret = 0;
    struct http_request *request = NULL;

    request = (struct http_request*)malloc(sizeof(struct http_request));
    if(request == NULL){
        return NULL;
    }

    /* parse initial line */
    line = evbuffer_readline(buf);
    ret = parse_initline(request,line);
    free(line);
    if(ret != 0){
        free(request);
        return NULL;
    }

    /* header queue */
    request->headers = hheader_new();
    if(request->headers == NULL){
        free(request);
        return NULL;
    }

    line = evbuffer_readline(buf);
    while(line != NULL && *line != '\0'){
        ret = hheader_add_line(request->headers,line);
        free(line);
        if(ret != 0){
            break;
        }
        line = evbuffer_readline(buf);
    }
    if(line != NULL){
        free(line);
    }

    request->is_cgi = 0;
    request->cgi_script = NULL;
    request->cgi_query_string = NULL;

    /* for cgi */
    if(request->method == REQ_POST){
        const char *len_str = hheader_get_value(request->headers,"Content-Length");
        int len = atoi(len_str);
        char *tmp = (char*)malloc(len + 1);
        
        request->is_cgi = 1;
        request->cgi_script = strdup(request->uri); /* the cgi script is the request uri */
        
        evbuffer_remove(buf,tmp,len); /* the cgi query string is in the optional body */
        tmp[len] = '\0';
        request->cgi_query_string = strdup(tmp);
        decode_query(request->cgi_query_string);

        free(tmp);
    }
    else if(request->method == REQ_GET){ /* the cgi script and query string is in the uri */
        /* is cgi pattern found in uri */
        int found = 0;
        char *tmp = strdup(hserver->cgi_pattern);
        char *token = strtok(tmp,";");
        while(token != NULL){
            if(strstr(request->uri,token)){
                found = 1;
                break;
            } 
            token = strtok(NULL,";");
        }
        free(tmp);

        if(found){ /* found cgi pattern in request uri */
            char *ch = strchr(request->uri,'?');
            if(ch != NULL){
                request->is_cgi = 1;
                *ch = '\0';
                request->cgi_script = strdup(request->uri);
                request->cgi_query_string = strdup(ch+1);
                decode_query(request->cgi_query_string);
                *ch = '?';
            }
        }
    }

    return request;
}

void hrequest_free(struct http_request *req)
{
    if(req == NULL) return;
    if(req->is_cgi){
        free(req->cgi_script);
        free(req->cgi_query_string);
    }
    free(req->uri);
    free(req->version);
    hheader_free(req->headers);
    free(req);
}
