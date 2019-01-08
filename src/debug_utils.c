/**
 * @file
 * @author Riccardo Mancini
 * 
 * @brief Implementation of debug_utils.h.
 * 
 * @see debug_utils.h
 */


#include "include/debug_utils.h"
#include "include/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/** LOG_LEVEL will be defined in another file */
extern const int LOG_LEVEL;


void dump_buffer_hex(char* buffer, int len){
  char *str, tmp[4];
  int i;

  str = malloc(len*3+1);

  str[0] = '\0';
  for (i=0; i<len; i++){
    sprintf(tmp, "%02x ", (unsigned char) buffer[i]);
    strcat(str, tmp);
  }

  LOG(LOG_DEBUG, "%s", str);
  free(str);
}
