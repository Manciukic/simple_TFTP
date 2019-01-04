/**
 * This file contains a macro for logging in different levels.
 *
 * Adapted from https://stackoverflow.com/a/328660
 */

#ifndef LOGGING
#define LOGGING

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_FATAL    (1)
#define LOG_ERR      (2)
#define LOG_WARN     (3)
#define LOG_INFO     (4)
#define LOG_DEBUG    (5)

#ifndef LOG_LEVEL
  #define LOG_LEVEL LOG_DEBUG
#endif

#define LOG(level, ...) do {  \
                          if (level <= LOG_LEVEL) { \
                            FILE *dbgstream; \
                            char where[25]; \
                            switch(level){ \
                              case LOG_FATAL: \
                                dbgstream = stderr; \
                                fprintf(dbgstream, "[FATAL]"); \
                                break; \
                              case LOG_ERR: \
                                dbgstream = stderr; \
                                fprintf(dbgstream, "[ERROR]"); \
                                break; \
                              case LOG_WARN: \
                                dbgstream = stderr; \
                                fprintf(dbgstream, "[WARN ]"); \
                                break; \
                              case LOG_INFO: \
                                dbgstream = stdout; \
                                fprintf(dbgstream, "[INFO ]"); \
                                break; \
                              case LOG_DEBUG: \
                                dbgstream = stdout; \
                                fprintf(dbgstream, "[DEBUG]"); \
                                break; \
                            } \
                            fprintf(dbgstream, "[%-5d]", (int) getpid()); \
                            snprintf(where, 25, "%s:%d", __FILE__, __LINE__); \
                            fprintf(dbgstream, " %-25s ", where); \
                            fprintf(dbgstream, __VA_ARGS__); \
                            fprintf(dbgstream, "\n"); \
                            fflush(dbgstream); \
                          } \
                        } while (0)

#endif
