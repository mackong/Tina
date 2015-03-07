/*
 * =====================================================================================
 *
 *       Filename:  util.h
 *
 *    Description:  some utilities.
 *
 *        Version:  1.0
 *        Created:  05/02/2012 10:32:33 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _UTIL_H
#define _UTIL_H

/**
  Get formatted string of current data and time.

  the formatted string is "%a, %d %b %Y %H:%M:%S GMT"

  @param buf pointer to a buffer for result string
  @param len length of result string buffer
  @return pointer to the result string buffer,NULL if error occured
 */
char *get_time_str(char *buf,size_t len);

/**
  get file size

  @param fp file stream pointer
  @return file size,-1 if error occured
 */
int get_file_size(FILE *fp);

/**
  create multilevel directory.

  @param path the multilevel directory path to create
  @result 0 for successful,-1 if error occured
 */
int mkdir_loop(const char *path);

/**
  simple to check the str is utf8 or not

  here just check (str -- str+8)
  @param str the string to check
  @return 1 for utf8 , 0 for not
 */
int is_utf8(char *str);

/**
  decode the uri string
  
  convert utf-8 string

  @param uri_str the uri string to decode
 */
void decode_uri( char* uri_str );

/**
  decode query string 

  @param query_str query string to decode
 */
void decode_query(char *query_str);

/**
  daemonize a process to be a daemon.

 */
void daemonize(void);

/**
  singleton a application.

  This make the Tina server to be only one 
  instance at any time.
 */
void singleton(void);

#endif
