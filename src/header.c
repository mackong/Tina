/*
 * =====================================================================================
 *
 *       Filename:  header.c
 *
 *    Description:  simple wrapper for http request/response header
 *
 *        Version:  1.0
 *        Created:  05/02/2012 09:45:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "header.h"

struct http_header_head *hheader_new()
{
    struct http_header_head *queue = (struct http_header_head*)malloc(sizeof(struct http_header_head));
    if(queue == NULL){
        return NULL;
    }

    TAILQ_INIT(queue);

    return queue;
}

void hheader_free(struct http_header_head *hqueue)
{
    struct http_header *header,*prev = NULL;

    TAILQ_FOREACH(header,hqueue,next){
    	if(prev != NULL) free(prev);
        free(header->name);
        free(header->value);
        prev = header;
    }
    if(prev != NULL) free(prev);
    free(hqueue);
}

int hheader_add_line(struct http_header_head *hqueue,char *line)
{
    char *token = NULL;
    struct http_header *header;

    if(hqueue == NULL || line == NULL){
        return -1;
    }

    header = (struct http_header*)malloc(sizeof(struct http_header));
    if(header == NULL){
        return -1;
    }

    /* name */
    token = strtok(line,":");
    header->name = strdup(token);

    /* value */
    token = strtok(NULL,":");
    if(token == NULL){
        header->value=strdup("");
    }
    else{
        while(*token == ' ')token++; /* until a non space character */
        header->value = strdup(token);
    }

    TAILQ_INSERT_TAIL(hqueue,header,next);

    return 0;
}

int hheader_add_pair(struct http_header_head *hqueue,char *name,char *value)
{
    struct http_header *header;

    if(hqueue == NULL || name == NULL || value == NULL){
        return -1;
    }

    header = (struct http_header*)malloc(sizeof(struct http_header));
    if(header == NULL){
        return -1;
    }

    /* name */
    header->name = strdup(name);
    
    /* value */
    header->value = strdup(value);

    TAILQ_INSERT_TAIL(hqueue,header,next);

    return 0;
}

const char *hheader_get_value(struct http_header_head *hqueue,const char *name)
{
    struct http_header *header;
    TAILQ_FOREACH(header,hqueue,next){
        if(strcmp(header->name,name) == 0){
            return header->value;
        }
    }
    return NULL;
}
