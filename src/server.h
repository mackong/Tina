/*
 * =====================================================================================
 *
 *       Filename:  server.h
 *
 *    Description:  http server.
 *
 *        Version:  1.0
 *        Created:  05/03/2012 04:05:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _SERVER_H
#define _SERVER_H

#include <sys/select.h>
#include <signal.h>
#include <netinet/in.h>

/** http server state  */
enum SERVER_STATE{
    SS_INIT, /*!< initial state  */
    SS_RUNNING, /*!< running state  */
    SS_STOP    /*!< stop state  */
};

/** http server  */
struct http_server{
    char *server_name; /*!< srever name */
    in_addr_t server_ip;   /*!< server ip */
    int server_port;   /*!< server port */
    int max_connection;/*!< max connection */
    char *default_page; /*!< default page */
    char *cgi_pattern; /*!< cgi pattern */
    char *root_document; /*!< root document */
    char *log_file;     /*!< log file */
    int connection_timeout; /*!< connection timeout */
    int list_directory;   /*!< list directory */

    volatile sig_atomic_t state;  /*!< server state */

    int sock_fd;         /*!< socket fd */
    int clients_fd[FD_SETSIZE]; /*!< connected clients fd */

    /** fd_set */
    fd_set read_set;
    fd_set all_set; 
};

/**
  Create a new http_server from a xml config file.
  
  This function must be call first before any other call.

  @param config_file xml config file
  @result pointer to the new http_server,NULL if error occured
 */
struct http_server *hserver_new(const char *config_file);

/**
  Start a http_server.

  @param hserver http_server to start
  @result 0 for successful,-1 if error occured
 */
int hserver_start(struct http_server *hserver);

/**
  Server mainloop

  This function process the clients' request.

  @param hserver pointer to http_server
 */
void hserver_mainloop(struct http_server *hserver);

/**
  Stop a http_server.

  @param hserver http_server to stop
  @result 0 for successful,-1 if error occured
 */
int hserver_stop(struct http_server *hserver);

/**
  Shutdown a http_server,release all resources

  This functio must be call last when no need the
  server to run.

  @param hserver http_server to shutdown
  @result 0 for successful,-1 if error occured
 */
int hserver_shutdown(struct http_server *hserver);

#endif
