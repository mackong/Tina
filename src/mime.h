/*
 * =====================================================================================
 *
 *       Filename:  mime.h
 *
 *    Description:  get mime type from mime_types of a given filename.
 *
 *        Version:  1.0
 *        Created:  05/02/2012 10:11:36 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

/**
  Get mime type of given filename.

  @param filename the filename to get mimetype
  @return pointer to mime type string,or NULL if error occured
 */
char *get_mime_type(const char *filename);
