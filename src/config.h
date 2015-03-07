/*
 * =====================================================================================
 *
 *       Filename:  config.h
 *
 *    Description:  default config for http server,if tina.conf doesn't configed,then use default config here.
 *
 *        Version:  1.0
 *        Created:  05/02/2012 09:30:15 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */


#ifndef _CONFIG_H
#define _CONFIG_H

/** server name */
#define SERVER_NAME "tina 0.1.0"

/** server ip */
#define SERVER_IP INADDR_ANY

/** server port */
#define SERVER_PORT 8080

/** max connection */
#define MAX_CONNECTION 1024

/** default page */
#define DEFAULT_PAGE "index.html"

/** cgi pattern */
#define CGI_PATTERN "cgi-bin"

/** 
  root document,
  don't append a /. 
 */
#define ROOT_DOCUMENT "/var/tina/www"

/**
   log file
   if it does not start with /,it is a relative path to root document
 */
#define LOG_FILE "log/tina.log"

/** connection timeout(seconds) */
#define CONNECTION_TIMEOUT 60

/** list directory */
#define LIST_DIRECTORY 0


/** file for lock*/
#define PIDFILE "var/run/tina.pid"

#endif
