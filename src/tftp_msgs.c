/**
 * @file
 * @author Riccardo Mancini
 * @brief Implementation of tftp_msgs.h .
 */

#include "tftp_msgs.h"
#include <string.h>
#include <stdio.h>

int tftp_msg_type(char *buffer){
  return (((int)buffer[0]) << 8) + buffer[1];
}

void tftp_msg_build_rrq(char* filename, char* mode, char* buffer){
  buffer[0] = 0;
  buffer[1] = 1;
  buffer += 2;
  strcpy(buffer, filename);
  buffer += strlen(filename)+1;
  strcpy(buffer, mode);
}

int tftp_msg_unpack_rrq(char* buffer, int buffer_len, char* filename, char* mode){
  int offset = 0;
  if (tftp_msg_type(buffer) != TFTP_TYPE_RRQ){
    return 1;
  }

  offset += 2;
  if (strlen(buffer+offset) > TFTP_MAX_FILENAME_LEN)
    return 3;
  strcpy(filename, buffer+offset);

  offset += strlen(filename)+1;
  if (strlen(buffer+offset) > TFTP_MAX_MODE_LEN)
    return 4;
  strcpy(mode, buffer+offset);

  offset += strlen(mode)+1;
  if (buffer_len != offset)
    return 2;

  if (strcmp(mode, TFTP_STR_NETASCII) == 0 || strcmp(mode, TFTP_STR_OCTET) == 0)
    return 0;
  else
    return 5;
}

int tftp_msg_get_size_rrq(char* filename, char* mode){
  return 4 + strlen(filename) + strlen(mode);
}

void tftp_msg_build_wrq(char* filename, char* mode, char* buffer){
  buffer[0] = 0;
  buffer[1] = 2;
  buffer += 2;
  strcpy(buffer, filename);
  buffer += strlen(filename)+1;
  strcpy(buffer, mode);
}

int tftp_msg_unpack_wrq(char* buffer, int buffer_len, char* filename, char* mode){
  int offset = 0;
  if (tftp_msg_type(buffer) != TFTP_TYPE_WRQ){
    return 1;
  }
  offset += 2;
  if (strlen(buffer+offset) > TFTP_MAX_FILENAME_LEN)
    return 3;
  strcpy(filename, buffer+offset);
  offset += strlen(filename)+1;
  if (strlen(buffer+offset) > TFTP_MAX_MODE_LEN)
    return 4;
  strcpy(mode, buffer+offset);
  offset += strlen(mode)+1;
  if (buffer_len != offset)
    return 2;
  if (strcmp(mode, TFTP_STR_NETASCII) == 0 || strcmp(mode, TFTP_STR_OCTET) == 0)
    return 0;
  else
    return 5;
}

int tftp_msg_get_size_wrq(char* filename, char* mode){
  return 4 + strlen(filename) + strlen(mode);
}

void tftp_msg_build_data(int block_n, char* data, int data_size, char* buffer){
  buffer[0] = 0;
  buffer[1] = 3;
  buffer[2] = block_n >> 8;
  buffer[3] = block_n;
  buffer += 4;
  memcpy(buffer, data, data_size);
}

int tftp_msg_unpack_data(char* buffer, int buffer_len, int* block_n, char* data, int* data_size){
  if (tftp_msg_type(buffer) != TFTP_TYPE_DATA){
    return 1;
  }
  if (buffer_len < 4)
    return 2;
  *block_n = (((int)buffer[2]) << 8) + buffer[3];
  *data_size = buffer_len - 4;
  if (*data_size > 0)
    memcpy(data, buffer+4, *data_size);
  return 0;
}

int tftp_msg_get_size_data(int data_size){
  return data_size + 4;
}

void tftp_msg_build_ack(int block_n, char* buffer){
  buffer[0] = 0;
  buffer[1] = 4;
  buffer[2] = block_n >> 8;
  buffer[3] = block_n;
}

int tftp_msg_unpack_ack(char* buffer, int buffer_len, int* block_n){
  if (tftp_msg_type(buffer) != TFTP_TYPE_ACK){
    return 1;
  }
  if (buffer_len != 4)
    return 2;
  *block_n = (((int)buffer[2]) << 8) + buffer[3];
  return 0;
}

int tftp_msg_get_size_ack(){
  return 4;
}

void tftp_msg_build_error(int error_code, char* error_msg, char* buffer){
  buffer[0] = 0;
  buffer[1] = 5;
  buffer[2] = error_code >> 8;
  buffer[3] = error_code;
  buffer += 4;
  strcpy(buffer, error_msg);
}

int tftp_msg_unpack_error(char* buffer, int buffer_len, int* error_code, char* error_msg){
  if (tftp_msg_type(buffer) != TFTP_TYPE_ERROR){
      return 1;
    }
    *error_code = (((int)buffer[2]) << 8) + buffer[3];
    if (*error_code < 0 || *error_code > 7)
      return 4;
    buffer += 4;
    if(strlen(buffer) > TFTP_MAX_ERROR_LEN)
      return 3;
    strcpy(error_msg, buffer);
    if (buffer_len != strlen(error_msg)+5)
      return 2;
    return 0;
}

int tftp_msg_get_size_error(char* error_msg){
  return 5 + strlen(error_msg);
}
