/*
 * =====================================================================================
 *
 *       Filename:  logger.c
 *
 *    Description:  a simple logger.
 *
 *        Version:  1.0
 *        Created:  05/02/2012 11:29:51 AM
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

#include "util.h"
#include "logger.h"

static FILE *log_fp = NULL;

int log_start(const char *log_file)
{
    char tmp[1024] = {'\0'};
    char *p = NULL;

    if(log_file == NULL){
        return -1;
    }

    strncpy(tmp,log_file,strlen(log_file));

    p = strrchr(tmp,'/');
    *p = '\0';
    if(mkdir_loop(tmp) != 0){
        perror("create log directory failed!\n");
        return -1;
    }
    *p = '/';

    log_fp = fopen(tmp,"a");
    if(log_fp == NULL){
        perror("open log file failed!\n");
        return -1;
    }

    return 0;
}

int log_write(int type,const char *msg)
{
    char time_str[50] = {'\0'};
    if(log_fp == NULL){
        perror("please open log first!\n");
        return -1;
    }

    if(get_time_str(time_str,sizeof(time_str)/sizeof(char)) == NULL){
        perror("get time string failed!\n");
        return -1;
    }

    if(fprintf(log_fp,"%s%s : %s%s\n",
            type == LOG_MESSAGE ? LOG_STR(LOG_MESSAGE) : LOG_STR(LOG_ERROR),
            time_str,msg,
            type == LOG_MESSAGE ? LOG_STR(LOG_MESSAGE) : LOG_STR(LOG_ERROR)) < 0){
        return -1;
    }
    
    fflush(NULL);

    return 0;
}

int log_shutdown()
{
    if(log_fp == NULL){
        perror("please open log first!\n");
        return -1;
    }

    fclose(log_fp);

    return 0;
}
