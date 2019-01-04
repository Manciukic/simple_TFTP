#include "include/debug_utils.h"
#include <stdio.h>
#include "include/logging.h"

void dump_buffer_hex(char* buffer, int len){
  for (int i=0; i<len; i++){
    LOG(LOG_DEBUG, "%d %x ", i, buffer[i]);
  }
}
