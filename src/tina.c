/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  main routine.
 *
 *        Version:  1.0
 *        Created:  05/03/2012 09:25:16 PM
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
#include <signal.h>
#include <string.h>
#include <getopt.h>

#include "util.h"
#include "server.h"

#define SOFTWARE_VERSION "beta 0.01"

static const char *optString = "c";
static const struct option longOpts[] = {
    {"config-file",required_argument,NULL,'c'},
    {"help",no_argument,NULL,0},
    {"version",no_argument,NULL,0},
    {NULL,no_argument,NULL,0}
};
static struct http_server *hserver = NULL;

void sig_handler(int arg)
{
    printf("%d stop\n",arg);
    hserver_stop(hserver);
    hserver_shutdown(hserver);
    exit(0);
}

int main(int argc,char **argv)
{
    int opt=0,longIndex=0;
    const char *default_config = "/etc/tina.conf";
    char *config_file = NULL;
    /* run in daemon process */
    daemonize();
    /* singleton the daemon process */
    singleton();

    while((opt=getopt_long(argc,argv,optString,longOpts,&longIndex)) != -1){
        switch(opt){
            case 'c':
                config_file = optarg;
                break;
            case 0:
                if(strcmp("help",longOpts[longIndex].name) == 0){
                    exit(0);
                }
                else if(strcmp("version",longOpts[longIndex].name) == 0){
                    exit(0);
                }
                break;
            default:
                exit(0);
        }
    }

    if(config_file == NULL){
        if((hserver = hserver_new(default_config)) == NULL){
            return 0;
        }
    }
    else{
        if((hserver = hserver_new(config_file)) == NULL){
            return 0;
        }
    }

    if(hserver_start(hserver) < 0){
        return 0;
    }

    signal(SIGTERM,sig_handler);
    signal(SIGINT,sig_handler);
    signal(SIGQUIT,sig_handler);
    signal(SIGPIPE,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);

    hserver_mainloop(hserver);

    return 0;
}
