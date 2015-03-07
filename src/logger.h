/*
 * =====================================================================================
 *
 *       Filename:  logger.h
 *
 *    Description:  a simple logger.
 *
 *        Version:  1.0
 *        Created:  05/02/2012 11:28:50 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  MacKong,mackonghp@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

/** log type */
enum {
    LOG_MESSAGE = 0, /*!< normal message  */
    LOG_ERROR = 1,   /*!< error message  */
};

/** log type string */
#define LOG_MESSAGE_STR "-----"
#define LOG_ERROR_STR   "*****"

/** convert message type to message type string */
#define LOG_STR(type) type##_STR


/**
  Start logger.

  This is must be called first.

  @param log_file log file name for logger
  @return 0 for successful,-1 if error occured
 */
int log_start(const char *log_file);

/**
  Write log message.

  @param type message type,MESSAGE or ERROR
  @param msg null terminated message string to write
  @return 0 for successful,-1 if error occured
 */
int log_write(int type,const char *msg);

/**
  Shutdown logger.
  This must be call last to release all resource.
  @return 0 for successful,-1 if error occured
 */
int log_shutdown();

