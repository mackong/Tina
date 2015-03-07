/*
 * =====================================================================================
 *
 *       Filename:  adder.c
 *
 *    Description:  a simple cgi program,add two number
 *
 *        Version:  1.0
 *        Created:  05/12/2012 07:51:50 PM
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
#include <string.h>

int main(void)
{
    char content[1024];
    char *buf,*p;
    int n1 = 0,n2 = 0;

    if((buf = getenv("QUERY_STRING")) != NULL){
        p = strchr(buf,'&');
        *p = '\0';
        n1 = atoi(buf);
        n2 = atoi(p+1);
    }

    sprintf(content,"Welcome to adder: ");
    sprintf(content,"%sThe Internet addition portal.\r\n<p>",content);
    sprintf(content,"%sThe answer is: %d + %d = %d\r\n<p>",content,n1,n2,n1+n2);
    sprintf(content,"%sThanks for visiting!\r\n",content);
    sprintf(content,"%s<hr>\r\nRequest Method: %s<br>\r\n",content,getenv("REQUEST_METHOD"));
    sprintf(content,"%sRemote Addr: %s<br>\r\n",content,getenv("REMOTE_ADDR"));
    sprintf(content,"%sHttp User Agent: %s<br>\r\n",content,getenv("HTTP_USER_AGENT"));

    printf("Content-Length: %d\r\n",(int)strlen(content));
    printf("Content-Type: text/html;charset=utf-8\r\n\r\n");
    printf("%s",content);
    fflush(stdout);

    exit(0);
}
