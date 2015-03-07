/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  http server.
 *
 *        Version:  1.0
 *        Created:  05/03/2012 04:05:39 PM
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
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <strings.h>  /* bzero */
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <pthread.h>

#include "config.h"
#include "logger.h"
#include "request.h"
#include "response.h"
#include "evbuffer.h"
#include "util.h"
#include "mime.h"
#include "server.h"

/* index into clients_fd array */
static int maxi = 0;
/* max available sockct fd */
static int maxfd = 0;
/* processed server */
static struct http_server *g_server = NULL;

/**
  Read xml config file to configure http_server.

  @param hserver http_server to configure
  @param config_file xml config file
  @result 0 for successful,-1 if error occured
 */
static int read_config(struct http_server *hserver,const char *config_file)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL;
    xmlNodePtr node = NULL;
    char *content = NULL;
    /* flags */
    int has_name = 0,has_ip = 0,has_port = 0;
    int has_maxconn = 0,has_defpage = 0,has_cgi = 0,has_rootdoc = 0,has_logfile = 0,has_timeout = 0,has_listdir = 0;

    doc = xmlReadFile(config_file,"UTF-8",XML_PARSE_NOBLANKS); /* ignore blanks */
    if(doc == NULL){ /* read file failed */
        return -1;
    }

    root = xmlDocGetRootElement(doc);
    if(root == NULL){  /* no root element */
        return -1;
    }

    if(xmlStrcmp(root->name,BAD_CAST("server_config")) != 0){  /* error document */
        return -1;
    }

    node = root->children;
    while(node != NULL){
        content = (char *)xmlNodeGetContent(node);
        if(xmlStrcmp(node->name,BAD_CAST("server_name")) == 0){
            has_name = 1;
            if(strcmp(content,"") == 0){
                hserver->server_name = strdup(SERVER_NAME); /* use default configure in config.h */
            }
            else{
                hserver->server_name = strdup(content);
            }
        }
        else if(xmlStrcmp(node->name,BAD_CAST("server_ip")) == 0){
            has_ip = 1;
            if(strcmp(content,"") == 0){
                hserver->server_ip = SERVER_IP;
            }
            else{
                hserver->server_ip = inet_addr(content);
            }
        }
        else if(xmlStrcmp(node->name,BAD_CAST("server_port")) == 0){
            has_port = 1;
            if(strcmp(content,"") == 0){
                hserver->server_port = SERVER_PORT;
            }
            else{
                hserver->server_port = atoi(content);
            }
        }
        else if(xmlStrcmp(node->name,BAD_CAST("max_connection")) == 0){
            has_maxconn = 1;
            if(strcmp(content,"") == 0){
                hserver->max_connection = MAX_CONNECTION;
            }
            else{
                hserver->max_connection = atoi(content);
            }
        }
        else if(xmlStrcmp(node->name,BAD_CAST("default_page")) == 0){
            has_defpage = 1;
            if(strcmp(content,"") == 0){
                hserver->default_page = strdup(DEFAULT_PAGE);
            }
            else{
                hserver->default_page = strdup(content);
            }
        }
        else if(xmlStrcmp(node->name,BAD_CAST("cgi_pattern")) == 0){
            has_cgi = 1;
            if(strcmp(content,"") == 0){
                hserver->cgi_pattern = strdup(CGI_PATTERN);
            }
            else{
                hserver->cgi_pattern = strdup(content);
            }
        }
        else if(xmlStrcmp(node->name,BAD_CAST("root_document")) == 0){
            has_rootdoc = 1;
            if(strcmp(content,"") == 0){
                hserver->root_document = strdup(ROOT_DOCUMENT);
            }
            else{
                hserver->root_document = strdup(content);
            }
        }
        else if(xmlStrcmp(node->name,BAD_CAST("log_file")) == 0){
            has_logfile = 1;
            if(strcmp(content,"") == 0){
                hserver->log_file = strdup(LOG_FILE);
            }
            else{
                hserver->log_file = strdup(content);
            }
        }
        else if(xmlStrcmp(node->name,BAD_CAST("connection_timeout")) == 0){
            has_timeout = 1;
            if(strcmp(content,"") == 0){
                hserver->connection_timeout = CONNECTION_TIMEOUT;
            }
            else{
                hserver->connection_timeout = atoi(content);
            }
        }
        else if(xmlStrcmp(node->name,BAD_CAST("list_directory")) == 0){
            has_listdir = 1;
            if(strcmp(content,"") == 0){
                hserver->list_directory = LIST_DIRECTORY;
            }
            else{
                hserver->list_directory = atoi(content);
            }
        }
        free(content);
        content = NULL;
        node = node->next;
    }

    xmlFreeDoc(doc);

    if(!has_name){
        hserver->server_name = strdup(SERVER_NAME);
    }
    if(!has_ip){
        hserver->server_ip = SERVER_IP;
    }
    if(!has_port){
        hserver->server_port = SERVER_PORT;
    }
    if(!has_maxconn){
        hserver->max_connection = MAX_CONNECTION;
    }
    if(!has_defpage){
        hserver->default_page = strdup(DEFAULT_PAGE);
    }
    if(!has_cgi){
        hserver->cgi_pattern = strdup(CGI_PATTERN);
    }
    if(!has_rootdoc){
        hserver->root_document = strdup(ROOT_DOCUMENT);
    }
    if(!has_logfile){
        hserver->log_file = strdup(LOG_FILE);
    }
    if(!has_timeout){
        hserver->connection_timeout = CONNECTION_TIMEOUT;
    }
    if(!has_listdir){
        hserver->list_directory = LIST_DIRECTORY;
    }

    return 0;
}

/**
  initial network for http server.

  Do the socket,bind,listen.

  @param hserver http_server to initial
  @result 0 for successful,-1 if error occured
 */
static int init_network(struct http_server *hserver)
{
    struct sockaddr_in servaddr;

    if((hserver->sock_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_IP)) < 0){
        return -1;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(hserver->server_ip);
    servaddr.sin_port = htons(hserver->server_port);

    if(bind(hserver->sock_fd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0){
        return -1;
    }

    if(listen(hserver->sock_fd,hserver->max_connection) < 0){
        return -1;
    }

    return 0;
}

struct http_server *hserver_new(const char *config_file)
{
    int i;
    struct http_server *hserver = NULL;

    hserver = (struct http_server*)malloc(sizeof(struct http_server));
    if(hserver == NULL){
        fprintf(stderr,"create http server failed!\n");
        return NULL;
    }

    if(read_config(hserver,config_file) < 0){
        fprintf(stderr,"read config file failed!\n");
        return NULL;
    }

    for(i = 0; i < FD_SETSIZE; i++){
        hserver->clients_fd[i] = -1;   /* -1 indicates avaiable entry */
    }

    FD_ZERO(&hserver->all_set);

    hserver->state = SS_INIT;

    return hserver;
}

int hserver_start(struct http_server *hserver)
{
    if(hserver->state == SS_INIT){/* the server is first to run,initial things */
        /* for first run,should setting these */
        /* network */
        if(init_network(hserver) < 0){
            fprintf(stderr,"network failed!\n");
            return -1;
        }

        /* logger */
        if(hserver->log_file[0] == '/'){ /* absolute path */
            if(log_start(hserver->log_file) < 0){
                fprintf(stderr,"start logger failed!\n");
            }
        }
        else{ /* relative path to root document */
            char buf[256] = {'\0'};
            strcpy(buf,hserver->root_document);
            strcat(buf,"/");
            strcat(buf,hserver->log_file);
            if(log_start(buf) < 0){
                fprintf(stderr,"start logger failed!\n");
            }
        }

        /* others */
        maxfd = hserver->sock_fd;
        maxi = -1;
        FD_SET(hserver->sock_fd,&hserver->all_set);
    }

    hserver->state = SS_RUNNING;

    g_server = hserver;

    if(log_write(LOG_MESSAGE,"Server start.") < 0){
        fprintf(stderr,"write log failed!\n");
    }

    return 0;
}

/**
  process client's request

  this is for pthread_create

  @param arg pointer to a client socket fd
  @result NULL
 */
void *client_process(void *arg)
{
    int fd = *((int*)arg);
    struct http_request *hrequest = NULL;
    struct evbuffer *inbuf = NULL;

    free(arg);
    
    inbuf = evbuffer_new();
    if(inbuf == NULL){
        if(log_write(LOG_ERROR,"evbuffer new failed") < 0){
            fprintf(stderr,"write log failed!\n");
        }
        return NULL;
    }

    /*while((bytes_read = evbuffer_read(inbuf,fd,4096)) > 0){ [> read all request data <]*/
        /*total_bytes += bytes_read;*/
    /*}*/

    if(evbuffer_read(inbuf,fd,4096) != 0){
        struct http_response *hresponse ;
        hrequest = hrequest_new(g_server,inbuf);
        hresponse = hresponse_new(g_server,hrequest);
        if(hresponse != NULL){
            hresponse_write(hresponse,fd);
            hresponse_free(hresponse);
        }
    }

    evbuffer_free(inbuf);
    hrequest_free(hrequest);

    if(close(fd) < 0){
        if(log_write(LOG_ERROR,"close file descriptor failed") < 0){
            fprintf(stderr,"write log failed!\n");
        }
    }

    return NULL;
}

void hserver_mainloop(struct http_server *hserver)
{
    char buf[256] = {'\0'};
    int i,nready;
    int connfd;
    socklen_t addrlen;
    struct sockaddr_in cliaddr;
    struct timeval timeout;


    while(hserver->state == SS_RUNNING){
        /* struct timeval */
        timeout.tv_sec = hserver->connection_timeout;
        timeout.tv_usec = 0;

        hserver->read_set = hserver->all_set; /* structure assignment */
        
        nready = select(maxfd + 1,&hserver->read_set,NULL,NULL,&timeout);
        if(nready < 0){
            if(log_write(LOG_ERROR,"select failed!") < 0){
                fprintf(stderr,"write log failed!\n");
            }
            continue;
        }
        else if(nready == 0){
            /* select timeout */
            /* fix me */
        }

        if(FD_ISSET(hserver->sock_fd,&hserver->read_set)){  /* new client connection */
            addrlen = sizeof(cliaddr);
            connfd = accept(hserver->sock_fd,(struct sockaddr*)&cliaddr,&addrlen);

            /* write connection log */
            sprintf(buf,"connection from %s:%d",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
            if(log_write(LOG_MESSAGE,buf) < 0){
                fprintf(stderr,"write log failed!\n");
            }

            for(i = 0; i < FD_SETSIZE; i++){
                if(hserver->clients_fd[i] < 0){
                    hserver->clients_fd[i] = connfd;    /* save descriptor */
                    break;
                }
            }

            if(i == FD_SETSIZE){
                fprintf(stderr,"Too many clients");
                continue;
            }

            FD_SET(connfd,&hserver->all_set); /* add new descriptor to set */

            if(connfd > maxfd){
                maxfd = connfd; /* for select */
            }
            if(i > maxi){
                maxi = i;  /* max index in client[] array */
            }
            if(--nready <= 0){
                continue; /* no more readable descriptors */
            }
        } /* if(FD_ISSET) */

        for(i = 0; i <= maxi; i++){ /* check all clients for data */
            int tempfd = hserver->clients_fd[i];
            if(tempfd < 0){
                continue;
            }
            if(FD_ISSET(tempfd,&hserver->read_set)){
                /*struct http_request *hrequest = NULL;*/
                /*struct evbuffer *inbuf = NULL;*/
                /*int total_bytes = 0,bytes_read = 0;*/
                /*inbuf = evbuffer_new();*/
                /*if(inbuf == NULL){*/
                    /*if(log_write(LOG_ERROR,"evbuffer new failed") < 0){*/
                        /*fprintf(stderr,"write log failed!\n");*/
                    /*}*/
                    /*return;*/
                /*}*/

                /*while((bytes_read = evbuffer_read(inbuf,tempfd,4096)) > 0){ [> read all request data <]*/
                    /*total_bytes += bytes_read;*/
                /*}*/
                /*if(evbuffer_read(inbuf,tempfd,4096) == 0){*/
                    /*if(close(tempfd) < 0){*/
                        /*if(log_write(LOG_ERROR,"close file descriptor failed") < 0){*/
                            /*fprintf(stderr,"write log failed!\n");*/
                        /*}*/
                        /*return ;*/
                    /*}*/
                    /*FD_CLR(tempfd,&hserver->all_set);*/
                    /*hserver->clients_fd[i] = -1;*/
                /*}*/
                /*else{*/
                    /*struct http_response *hresponse ;*/
                    /*hrequest = hrequest_new(inbuf);*/
                    /*hresponse = hresponse_new(hserver,hrequest);*/
                    /*if(hresponse != NULL){*/
                        /*hresponse_write(hresponse,tempfd);*/
                        /*hresponse_free(hresponse);*/
                    /*}*/
                /*}*/

                /*evbuffer_free(inbuf);*/
                /*hrequest_free(hrequest);*/

                /* create thread to process client request */
                pthread_t pid;
                pthread_attr_t attr;
                int *pfd = (int*)malloc(sizeof(int));
                *pfd = tempfd;
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
                pthread_create(&pid,&attr,client_process,(void*)pfd);
                pthread_attr_destroy(&attr);

                FD_CLR(tempfd,&hserver->all_set);
                hserver->clients_fd[i] = -1;
                if(--nready <= 0){
                    break;
                }
            }
        } /* for(i = 0; i <= maxi; i++) */
    } /* while */
}

int hserver_stop(struct http_server *hserver)
{
    if(hserver->state != SS_RUNNING){ /* server is not running */
        fprintf(stderr,"Server is not running!\n");
        return -1;
    }

    hserver->state = SS_STOP; /* the server thread will be exit */

    if(log_write(LOG_MESSAGE,"Server stoped!") < 0){
        fprintf(stderr,"write log failed!\n");
    }

    return 0;
}

int hserver_shutdown(struct http_server *hserver)
{
    if(hserver == NULL){
        return -1;
    }

    if(hserver->state == SS_RUNNING){
        hserver_stop(hserver);
    }

    close(hserver->sock_fd);
    free(hserver->server_name);
    free(hserver->default_page);
    free(hserver->cgi_pattern);
    free(hserver->root_document);
    free(hserver->log_file);
    free(hserver);
    hserver = NULL;
    g_server = NULL;
    
    return 0;
}
