/*
 * =====================================================================================
 *
 *       Filename:  response.c
 *
 *    Description:  simple wrapper for http response.
 *
 *        Version:  1.0
 *        Created:  05/09/2012 01:59:14 PM
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
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h> /* getpeername */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netdb.h>
#include <dirent.h>
#include <limits.h>

#include "server.h"
#include "request.h"
#include "logger.h"
#include "util.h"
#include "evbuffer.h"
#include "mime.h"
#include "response.h"

extern char **environ;

/**
  check the http request bad or not.

  here just check if the HTTP/1.1 request have a host header or not.

  @param hrequest the http request to check
  @return 1 if the request is a bad request,0 otherwise
 */
static int req_badcheck(struct http_request *hrequest)
{
    if(strcmp(hrequest->version,"HTTP/1.0") == 0){
        return 0;
    }
    else if(strcmp(hrequest->version,"HTTP/1.1") == 0){ /* HTTP/1.1 must have Host header */
        if(hheader_get_value(hrequest->headers,"Host") == NULL){
            return 1;
        }
        return 0;
    }
    
    /* anything other is bad */
    return 1;
}

/**
  check whether the file path is exist or is it can be access

  @param path the path to check
  @return 0 for ok,404 for not exist,403 for can not be access
 */
static int path_check(const char *path)
{
    if(access(path,F_OK) != 0){
        return RESPONSE_NOTFOUND;
    }
    
    if(access(path,R_OK) != 0){
        return RESPONSE_FORBIDDEN;
    }
    return 0;
}

/**
  get status code and message for a http response

  this function also get the decoded uri for the response

  @param hresponse the http response to get for
  @param hserver the http server
  @param hrequest the http request
  @return the status code
 */
static int get_status(struct http_response *hresponse,struct http_server *hserver,struct http_request *hrequest)
{
    char temp[1024] = {'\0'};
    int code = 0;

    /* for bad request */
    if(req_badcheck(hrequest)){
        hresponse->status_code = RESPONSE_BADREQ;
        hresponse->status_msg = strdup(CODE_2_MSG(RESPONSE_BADREQ));
        sprintf(temp,"%s%s%d.html",hserver->root_document,"/error/",RESPONSE_BADREQ);
        hresponse->decoded_uri = strdup(temp);
        return RESPONSE_BADREQ;
    }

    if(strcmp(hrequest->uri,"/") == 0){ /* default page */
        char *default_page = NULL;
        char *pages = strdup(hserver->default_page);
        default_page = strtok(pages,";");
        while(default_page != NULL){ /* loop each page in server's default_page */
            memset(temp,'\0',sizeof(temp));
            sprintf(temp,"%s/%s",hserver->root_document,default_page);
            if((code = path_check(temp)) == 0){
                break;
            }
            default_page = strtok(NULL,";");
        }/* while */
        free(pages);
    } /* default page */
    else{ /* not default page */
        if(hrequest->is_cgi){
            sprintf(temp,"%s%s",hserver->root_document,hrequest->cgi_script);
        }
        else{
            sprintf(temp,"%s%s",hserver->root_document,hrequest->uri);
        }
        decode_uri(temp);
        code = path_check(temp);
    }

    if(code != 0){ /* not found or forbidden */
        hresponse->status_code = code;
        if(code == RESPONSE_FORBIDDEN){
            hresponse->status_msg = strdup(CODE_2_MSG(RESPONSE_FORBIDDEN));
        }
        else{
            hresponse->status_msg = strdup(CODE_2_MSG(RESPONSE_NOTFOUND));
        }

        memset(temp,'\0',sizeof(temp));
        sprintf(temp,"%s%d.html","etc/tina/error/",code);
        hresponse->decoded_uri = strdup(temp);
        
        return code;
    }

    /* for not implemented */
    if(hrequest->method == REQ_UNKNOWN){
        hresponse->status_code = RESPONSE_NOTIMP;
        hresponse->status_msg = strdup(CODE_2_MSG(RESPONSE_NOTIMP));

        memset(temp,'\0',sizeof(temp));
        sprintf(temp,"%s%s%d.html",hserver->root_document,"/error/",RESPONSE_NOTIMP);
        hresponse->decoded_uri = strdup(temp);

        return RESPONSE_NOTIMP;
    }

    /* ok */
    hresponse->status_code = RESPONSE_OK;
    hresponse->status_msg = strdup(CODE_2_MSG(RESPONSE_OK));
    hresponse->decoded_uri = strdup(temp);
    return RESPONSE_OK;
}

/* used for file size */
#define B 1
#define KB 1024
#define MB 1024*1024
#define GB 1024*1024*1024
/**
  list the direcoty for a http response

  @param hresponse the result http response
 */
static void list_dir(struct http_response *hresponse)
{
    /* is the last character or request uri is / or not */
    int has_slash = 0;
    struct dirent **entrylist;
    char pathname[PATH_MAX + 1] = {'\0'};
    char date_buf[50] = {'\0'};
    struct tm *tmp = NULL;
    int i = 0,total,entryIndex = 0;
    long long size_arr[] = {GB,MB,KB,B};
    char* desc_arr[] = {"GB","MB","KB","B"};
    char size_buf[32] = {'\0'};

    
    evbuffer_add_printf(hresponse->body_buf,"%s","<html>\r\n");
    evbuffer_add_printf(hresponse->body_buf,"<head> \
            <meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8 \" /> \
            <title>Index of %s</title></head>\r\n",hresponse->req_uri);
    evbuffer_add_printf(hresponse->body_buf,"%s",
            "<body bgcolor=\"#99cc99\" text=\"#000000\" link=\"#2020ff\" vlink=\"#4040cc\">\r\n");
    evbuffer_add_printf(hresponse->body_buf,"<h2>Index of %s</h2>\r\n",hresponse->req_uri);
    evbuffer_add_printf(hresponse->body_buf,"%s\r\n","<hr />");

    total = scandir(hresponse->decoded_uri,&entrylist,0,alphasort);

    if(total < 0){
        evbuffer_add_printf(hresponse->body_buf,"read %s failed\r\n",hresponse->decoded_uri);
    }
    else{
        /* write a html table */
        evbuffer_add_printf(hresponse->body_buf,"%s\r\n","<table cellpadding=\"4\" border=\"0\">");
        evbuffer_add_printf(hresponse->body_buf,"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
                "<tr>","<th align=\"left\">Name</th>","<th align=\"right\">Size</th>",
                "<th align=\"right\">Date Modified</th>","</tr>");

        while(entryIndex < total){
            struct stat entryInfo;
            if(hresponse->req_uri[strlen(hresponse->req_uri) - 1] == '/'){
                has_slash = 1;
                sprintf(pathname,"%s%s",hresponse->decoded_uri,entrylist[entryIndex]->d_name);
            }
            else{
                has_slash = 0;
                sprintf(pathname,"%s/%s",hresponse->decoded_uri,entrylist[entryIndex]->d_name);
            }

            if( (strcmp(entrylist[entryIndex]->d_name,".") == 0) || 
                (lstat(pathname,&entryInfo) != 0) ){
                entryIndex++;
                continue;
            }

            evbuffer_add_printf(hresponse->body_buf,"%s\r\n","<tr>");

            /* Name and Size*/
            if(S_ISDIR(entryInfo.st_mode)){ /* for directory ,we add a / after the name,and no size */
                evbuffer_add_printf(hresponse->body_buf,"<td align=\"left\"> \
                        <a href=\"%s%s%s\">%s/</a></td>\r\n",
                        hresponse->req_uri,has_slash?"":"/",entrylist[entryIndex]->d_name,
                        (strcmp(entrylist[entryIndex]->d_name,"..")==0)?"[parent directory]":entrylist[entryIndex]->d_name);

                evbuffer_add_printf(hresponse->body_buf,"<td align=\"right\">%s</td>","");
            }
            else{
                evbuffer_add_printf(hresponse->body_buf,"<td align=\"left\"> \
                        <a href=\"%s%s%s\">%s</a></td>\r\n",
                        hresponse->req_uri,has_slash?"":"/",entrylist[entryIndex]->d_name,entrylist[entryIndex]->d_name);

                for(i = 0; i < sizeof(size_arr)/sizeof(long long); i++){
                    if(entryInfo.st_size > size_arr[i]) break;
                }

                if(i == sizeof(size_arr)/sizeof(long long)){
                    sprintf(size_buf,"%d B",0);
                }
                else{
                    sprintf(size_buf,"%.1f %s",(double)entryInfo.st_size/(double)size_arr[i],desc_arr[i]);
                }

                evbuffer_add_printf(hresponse->body_buf,"<td align=\"right\">%s</td>",size_buf);
            }
            
            /* Date Modified */
            tmp = localtime(&entryInfo.st_mtime);
            strftime(date_buf,sizeof(date_buf)/sizeof(char),"%D %r",tmp);
            evbuffer_add_printf(hresponse->body_buf,"<td align=\"right\">%s</td>",date_buf);

            evbuffer_add_printf(hresponse->body_buf,"%s\r\n","</tr>");
            free(entrylist[entryIndex]);
            entryIndex++;
        }
        free(entrylist);
        evbuffer_add_printf(hresponse->body_buf,"%s\r\n","</table>");
    }

    evbuffer_add_printf(hresponse->body_buf,"%s\r\n%s\r\n","</body>","</html>");

    sprintf(size_buf,"%d",EVBUFFER_LENGTH(hresponse->body_buf));
    hheader_add_pair(hresponse->headers,"Content-Length",size_buf);

    hheader_add_pair(hresponse->headers,"Content-Type","text/html");
}

/**
  process static request

  @param hresponse the result http response
  @param hrequest the http request
  @param hserver the http server
  @param 0 for process successful,-1 if error occured
 */
static int hresponse_static(struct http_response *hresponse,struct http_request *hrequest,struct http_server *hserver)
{
    int file_size = 0;
    FILE *fp = NULL;
    char *mime_type = NULL;
    char size_buf[32] = {'\0'};
    char time_str[50] = {'\0'};
    char *data = NULL;
    /* for last modified time */
    struct stat stat_buf;
    struct tm *tmp;

    hresponse->type = 0;

    fp = fopen(hresponse->decoded_uri,"r");
    if(fp == NULL){
        return -1;
    }
    /* file size */
    file_size = get_file_size(fp);
    if(file_size < 0){
        return -1;
    }

    /* headers */
    hheader_add_pair(hresponse->headers,"Connection","close");
    hheader_add_pair(hresponse->headers,"Server",hserver->server_name);

    hheader_add_pair(hresponse->headers,"Date",get_time_str(time_str,sizeof(time_str)/sizeof(char)));
   
    /* last modified time */
    stat(hresponse->decoded_uri,&stat_buf);
    tmp = localtime(&stat_buf.st_mtime);
    strftime(time_str,sizeof(time_str)/sizeof(char),"%a, %d %b %Y %H:%M:%S GMT",tmp);
    hheader_add_pair(hresponse->headers,"Last-Modified",time_str);

    /* body content */
    if(hrequest->method == REQ_GET){
        if((stat_buf.st_mode & S_IFMT) == S_IFREG){ /* regular file */
            sprintf(size_buf,"%d",file_size);
            hheader_add_pair(hresponse->headers,"Content-Length",size_buf);

            mime_type = get_mime_type(hresponse->decoded_uri);
            hheader_add_pair(hresponse->headers,"Content-Type",mime_type == NULL?"text/plain;charset=utf-8":mime_type);

            data = (char*)malloc(sizeof(char) * file_size);
            fread(data,file_size,sizeof(char),fp);
            evbuffer_add(hresponse->body_buf,data,file_size);
            free(data);
        }
        else if((stat_buf.st_mode & S_IFMT) == S_IFDIR){ /* directory */
            if(!hserver->list_directory){ /* if list directory function is closed,just return a simple error message */
                hresponse->status_code = RESPONSE_FORBIDDEN;
                hresponse->status_msg = strdup(CODE_2_MSG(RESPONSE_FORBIDDEN));
                evbuffer_add_printf(hresponse->body_buf,"%s\r\n","The list directory function is closed by Tina server");
                evbuffer_add_printf(hresponse->body_buf,"%s\r\n\r\n","You can contact the admin to solve this problem");
                evbuffer_add_printf(hresponse->body_buf,"%s\r\n","Tina Web Server");

                sprintf(size_buf,"%d",EVBUFFER_LENGTH(hresponse->body_buf));
                hheader_add_pair(hresponse->headers,"Content-Length",size_buf);

                hheader_add_pair(hresponse->headers,"Content-Type","text/plain;charset=utf-8");
            }
            else{
                list_dir(hresponse);
            }
        }
    }

    return 0;
}

/**
  process cgi request

  here we just add some header information and some environment variables,
  when call hrespose_write,we will create child process to run cgi program

  @param hresponse the result http response
  @param hrequest the http request
  @param hserver http server
  @return 0 for process successful,-1 if error occured
 */
static int hresponse_dynamic(struct http_response *hresponse,struct http_request *hrequest,struct http_server *hserver)
{
    char time_str[50] = {'\0'};
    struct stat stat_buf;
    struct tm *tmp;

    hresponse->type = 1;

    /* headers */
    hheader_add_pair(hresponse->headers,"Connection","close");
    hheader_add_pair(hresponse->headers,"Server",hserver->server_name);

    hheader_add_pair(hresponse->headers,"Date",get_time_str(time_str,sizeof(time_str)/sizeof(char)));
   
    /* last modified time */
    stat(hresponse->decoded_uri,&stat_buf);
    tmp = localtime(&stat_buf.st_mtime);
    strftime(time_str,sizeof(time_str)/sizeof(char),"%a, %d %b %Y %H:%M:%S GMT",tmp);
    hheader_add_pair(hresponse->headers,"Last-Modified",time_str);
  
    return 0;
}

struct http_response *hresponse_new(struct http_server *hserver,struct http_request *hrequest)
{
    struct http_response *hresponse = NULL;

    if(hserver == NULL || hrequest == NULL){
        goto FAIL;
    }

    hresponse = (struct http_response*)malloc(sizeof(struct http_response));
    if(hresponse == NULL){
        goto FAIL;
    }
    hresponse->headers = hheader_new();
    if(hresponse->headers == NULL){
        free(hresponse);
        goto FAIL;
    }
    hresponse->body_buf = evbuffer_new();
    if(hresponse->body_buf == NULL){
        hheader_free(hresponse->headers);
        free(hresponse);
        goto FAIL;
    }

    hresponse->hrequest = hrequest;
    hresponse->hserver = hserver;

    /* response initial line */
    hresponse->version = strdup(hrequest->version);
    hresponse->req_uri = strdup(hrequest->uri);
    decode_uri(hresponse->req_uri);
    get_status(hresponse,hserver,hrequest);

    /* header and body */
    if(!hrequest->is_cgi || hresponse->status_code != RESPONSE_OK){ /* not cgi request */
        if(hresponse_static(hresponse,hrequest,hserver) < 0){
            hresponse_free(hresponse);
            goto FAIL;
        }
    }
    else{
        if(hresponse_dynamic(hresponse,hrequest,hserver) < 0){
            hresponse_free(hresponse);
            goto FAIL;
        }
    }

    return hresponse;
FAIL:
    if(log_write(LOG_ERROR,"create response failed") < 0){
        fprintf(stderr,"write log failed!\n");
    }
    return NULL;
}

int hresponse_write(struct http_response *hresponse,int connfd)
{
    struct evbuffer *outbuf;
    struct http_header *header;

    outbuf = evbuffer_new();
    if(outbuf == NULL){
        return -1;
    }

    /* add initial line */
    evbuffer_add_printf(outbuf,"%s %d %s\r\n",hresponse->version,hresponse->status_code,hresponse->status_msg);
    
    /* add headers */
    TAILQ_FOREACH(header,hresponse->headers,next){
        evbuffer_add_printf(outbuf,"%s:%s\r\n",header->name,header->value);
    }

    if(hresponse->type == 0){ /* static response */
        evbuffer_add(outbuf,"\r\n",2);
        evbuffer_add_buffer(outbuf,hresponse->body_buf);
        evbuffer_write(outbuf,connfd);
    }
    else{ /* dynamic response */
        char *emptyarg[] = {NULL};
        
        evbuffer_write(outbuf,connfd);

        if(fork() == 0){ /* child process for cgi */
            char tmp[32] = {'\0'};
            char sname[255];
            struct sockaddr_in client_addr;
            socklen_t addr_len;

            /* get remote info */
            addr_len = sizeof(client_addr);
            getpeername(connfd,(struct sockaddr*)&client_addr,&addr_len);

            /* QUERY_STRING */
            setenv("QUERY_STRING",hresponse->hrequest->cgi_query_string,1);

            /* SERVER_NAME */
            gethostname(sname,sizeof(sname));
            setenv("SERVER_NAME",sname,1);

            /* SERVER_SOFTWARE */
            setenv("SERVER_SOFTWARE",hresponse->hserver->server_name,1);

            /* SERVER_PORT */
            sprintf(tmp,"%d",hresponse->hserver->server_port);
            setenv("SERVER_PORT",tmp,1);
            
            /* SERVER_PROTOCOL */
            setenv("SERVER_PROTOCOL",hresponse->hrequest->version,1);

            /* SCRIPT_NAME */
            setenv("SCRIPT_NAME",hresponse->hrequest->cgi_script,1);

            /* GATEWAY_INTERFACE */
            setenv("GATEWAY_INTERFACE","CGI/1.1",1);

            /* HTTP_ACCEPT */
            setenv("HTTP_ACCEPT",hheader_get_value(hresponse->hrequest->headers,"Accept"),1);

            /* HTTP_ACCEPT_ENCODING */
            setenv("HTTP_ACCEPT_ENCODING",hheader_get_value(hresponse->hrequest->headers,"Accept-Encoding"),1);

            /* HTTP_ACCEPT_LANGUAGE */
            setenv("HTTP_ACCEPT_LANGUAGE",hheader_get_value(hresponse->hrequest->headers,"Accept-Language"),1);

            /* HTTP_CONNECTION */
            setenv("HTTP_CONNECTION",hheader_get_value(hresponse->hrequest->headers,"Connection"),1);
            
            /* REQUEST_METHOD */
            setenv("REQUEST_METHOD",hresponse->hrequest->method == REQ_GET?"GET":"POST",1);

            /* REMOTE_ADDR */
            setenv("REMOTE_ADDR",inet_ntoa(client_addr.sin_addr),1);

            /* HTTP_USER_AGENT */
            setenv("HTTP_USER_AGENT",hheader_get_value(hresponse->hrequest->headers,"User-Agent"),1);

            if(hresponse->hrequest->method == REQ_POST){
                setenv("CONTENT_TYPE",hheader_get_value(hresponse->hrequest->headers,"Content-Type"),1);
                setenv("CONTENT_LENGTH",hheader_get_value(hresponse->hrequest->headers,"Content-Length"),1);
            }

            dup2(connfd,STDOUT_FILENO);
            execve(hresponse->decoded_uri,emptyarg,environ);
        }
        wait(NULL);
    }

    evbuffer_free(outbuf);
    return 0;
}

int hresponse_free(struct http_response *hresponse)
{
    if(hresponse == NULL) return -1;
    
    free(hresponse->version);
    free(hresponse->status_msg);
    free(hresponse->req_uri);
    free(hresponse->decoded_uri);
    hheader_free(hresponse->headers);
    evbuffer_free(hresponse->body_buf);
    free(hresponse);
    hresponse = NULL;

    return 0;
}
