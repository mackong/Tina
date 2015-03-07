/*
 * =====================================================================================
 *
 *       Filename:  util.c
 *
 *    Description:  some utilities.
 *
 *        Version:  1.0
 *        Created:  05/02/2012 10:33:51 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"

char *get_time_str(char *buf,size_t len)
{
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);
    if(tmp == NULL){
        return NULL;
    }

    if(strftime(buf,len,"%a, %d %b %Y %H:%M:%S GMT",tmp) == 0){
        return NULL;
    }

    return buf;
}

int get_file_size(FILE *fp)
{
    int size = 0;
    if(fseek(fp,0,SEEK_END) < 0){
        return -1;
    }
    size = ftell(fp);
    if(fseek(fp,0,SEEK_SET) < 0){
        return -1;
    }
    return size;
}

int mkdir_loop(const char *path)
{
    char tmp[1024] = {'\0'};
    int i,len;

    if( (len = strlen(path)) == 0 || path == NULL){
        return -1;
    }

    strncpy(tmp,path,len);

    for(i = 1; i < len; i++){
        if(tmp[i] == '/'){
            tmp[i] = '\0';
            if(access(tmp,F_OK) != 0){
                mkdir(tmp,0777);
            }
            tmp[i] = '/';
        }
    }

    if(len > 0 && access(tmp,F_OK) != 0){
        mkdir(tmp,0777);
    }
    return 0;
}

int is_utf8(char *str)
{
    /* utf8: %Ex%xx%xx */
    char *p =strstr(str, "%E");

    if(p ==NULL || strlen(p) <9 || *(p+3) != '%' || *(p+6) !='%'){
        return 0;
    }
    if(p >= str+3){
        if(*(p-3) =='%' && (*(p-2) < '1' || *(p-2) > '3')){
            return 0;
        }
    }
    return 1;
}

/* convert hex char to decimal number */
#define HEX2NUM(c)(unsigned char)((c<='9')?c-'0':(c<='F')?c-'A'+10:c-'a'+10)

/**
  decode helper function

  @param str the string to decode
  @param flag 1 for decode '+',0 for not decode '+'
 */
static void decode_helper(char *str,int flag)
{
    char *s,*d;
    if(!str) return;
    for(s=d=str; *s; s++){
        if(flag && *s == '+'){
            *d++ = ' ';
        }
        else if(*s == '%'){
            if(is_utf8(s)){ /* utf-8 string */
                int i = 0;
                for(i = 0; i < 9; i += 3){ /* convert utf8 */
                    unsigned char b = HEX2NUM(*(s+1))<<4 | HEX2NUM(*(s+2));
                    *d++ = b;
                    s+=3;
                }
                s-=1; /*before minus 1,s is pointer to %,so minus 1 make s pointer to the last x */
            }
            else{ /* a special char such as %3F=?*/
                char tmp[] = {*(s+1),*(s+2),'\0'};
                *d++ = (char)strtol(tmp,NULL,16);
                s+=2;
            }
        }
        else{
            *d++ = *s;
        }
    }
    *d = 0;
}

void decode_uri( char* uri_str )
{
    decode_helper(uri_str,0);
}

void decode_query(char *query_str)
{
    decode_helper(query_str,1);
}

void daemonize(void)
{
        pid_t pid;
        pid = fork();
        if (pid < 0) {
                fprintf(stderr, "fork, fail.\n");
                exit(1);
        }
 
        if (pid > 0) {
                exit(0);
        }
 
        setsid();
        
        pid = fork();
        if (pid < 0) {
                fprintf(stderr, "fork, fail.\n");
                exit(1);
        }
 
        if (pid > 0) {
                exit(0);
        }
         
        close(0);
        close(1);
        close(2);
 
        chdir("/");
        umask(0);
}

/**
  lock a file description

  @param fd the file description to lock
  @result 0 for lock successful,otherwise fail
 */
static int lockfile(const int fd)
{
    struct flock stLock;

    stLock.l_type = F_WRLCK;
    stLock.l_start = 0;
    stLock.l_whence = SEEK_SET;
    stLock.l_len = 0;

    return fcntl(fd,F_SETLK,&stLock);
}

/**
  is the application is running already.

  @result 0 for not running,1 for running already,-1 for lock pid file fail
 */
static int is_running()
{
    char buf[16];
    const int fd = open(PIDFILE,O_RDWR|O_CREAT,(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));
    if(fd < 0){
        return -1;
    }

    /* lock file */
    if(lockfile(fd) < 0){
        if(errno == EACCES || errno == EAGAIN){
            close(fd);
            return 1; /* already running */
        }
        return -1; /* fail to lock */
    }

    /* write pid */
    ftruncate(fd,0);
    sprintf(buf,"%ld",(long)getpid());
    write(fd,buf,strlen(buf)+1);

    return 0;
}

void singleton()
{
    if(is_running() != 0){
        exit(1);
    }
}
