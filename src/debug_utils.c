#include "debug_utils.h"
#include <stdio.h>

void dump_buffer_hex(char* buffer, int len){
  for (int i=0; i<len; i++){
    printf("%x ", buffer[i]);
  }
  printf("\n");
}
