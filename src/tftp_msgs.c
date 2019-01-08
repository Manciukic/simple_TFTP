/**
 * @file
 * @author Riccardo Mancini
 * @brief Implementation of tftp_msgs.h .
 * 
 * @see tftp_msgs.h
 */


/** Defines log level to this file. */
#define LOG_LEVEL LOG_INFO


#include "include/tftp_msgs.h"
#include "include/logging.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdint.h>


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
    LOG(LOG_ERR, "Expected RRQ message (1), found %d", tftp_msg_type(buffer));
    return 1;
  }

  offset += 2;
  if (strlen(buffer+offset) > TFTP_MAX_FILENAME_LEN){
    LOG(LOG_ERR, "Filename too long (%d > %d): %s", (int) strlen(buffer+offset), TFTP_MAX_FILENAME_LEN, buffer+offset);
    return 3;
  }
  strcpy(filename, buffer+offset);

  offset += strlen(filename)+1;
  if (strlen(buffer+offset) > TFTP_MAX_MODE_LEN){
    LOG(LOG_ERR, "Mode string too long (%d > %d): %s", (int) strlen(buffer+offset), TFTP_MAX_MODE_LEN, buffer+offset);
    return 4;
  }
  strcpy(mode, buffer+offset);

  offset += strlen(mode)+1;
  if (buffer_len != offset){
    LOG(LOG_ERR, "Packet contains unexpected fields");
    return 2;
  }
  if (strcmp(mode, TFTP_STR_NETASCII) == 0 || strcmp(mode, TFTP_STR_OCTET) == 0)
    return 0;
  else{
    LOG(LOG_ERR, "Unrecognized transfer mode: %s", mode);
    return 5;
  }
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
    LOG(LOG_ERR, "Expected WRQ message (2), found %d", tftp_msg_type(buffer));
    return 1;
  }

  offset += 2;
  if (strlen(buffer+offset) > TFTP_MAX_FILENAME_LEN){
    LOG(LOG_ERR, "Filename too long (%d > %d): %s", (int) strlen(buffer+offset), TFTP_MAX_FILENAME_LEN, buffer+offset);
    return 3;
  }

  strcpy(filename, buffer+offset);
  offset += strlen(filename)+1;
  if (strlen(buffer+offset) > TFTP_MAX_MODE_LEN){
    LOG(LOG_ERR, "Mode string too long (%d > %d): %s", (int) strlen(buffer+offset), TFTP_MAX_MODE_LEN, buffer+offset);
    return 4;
  }

  strcpy(mode, buffer+offset);
  offset += strlen(mode)+1;
  if (buffer_len != offset){
    LOG(LOG_ERR, "Packet contains unexpected fields");
    return 2;
  }

  if (strcmp(mode, TFTP_STR_NETASCII) == 0 || strcmp(mode, TFTP_STR_OCTET) == 0)
    return 0;
  else{
    LOG(LOG_ERR, "Unrecognized transfer mode: %s", mode);
    return 5;
  }
}


int tftp_msg_get_size_wrq(char* filename, char* mode){
  return 4 + strlen(filename) + strlen(mode);
}


void tftp_msg_build_data(int block_n, char* data, int data_size, char* buffer){
  buffer[0] = 0;
  buffer[1] = 3;
  *((uint16_t*)(buffer+2)) = htons((uint16_t) block_n);
  buffer += 4;
  memcpy(buffer, data, data_size);
}


int tftp_msg_unpack_data(char* buffer, int buffer_len, int* block_n, char* data, int* data_size){
  if (tftp_msg_type(buffer) != TFTP_TYPE_DATA){
    LOG(LOG_ERR, "Expected DATA message (3), found %d", tftp_msg_type(buffer));
    return 1;
  }

  if (buffer_len < 4){
    LOG(LOG_ERR, "Packet size too small for DATA: %d > 4", buffer_len);
    return 2;
  }

  *block_n = (int) ntohs(*((uint16_t*)(buffer+2)));
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
  *((uint16_t*)(buffer+2)) = htons((uint16_t) block_n);
}


int tftp_msg_unpack_ack(char* buffer, int buffer_len, int* block_n){
  if (tftp_msg_type(buffer) != TFTP_TYPE_ACK){
    LOG(LOG_ERR, "Expected ACK message (4), found %d", tftp_msg_type(buffer));
    return 1;
  }

  if (buffer_len != 4){
    LOG(LOG_ERR, "Wrong packet size for ACK: %d != 4", buffer_len);
    return 2;
  }
  *block_n = (int) ntohs(*((uint16_t*)(buffer+2)));
  return 0;
}


int tftp_msg_get_size_ack(){
  return 4;
}


void tftp_msg_build_error(int error_code, char* error_msg, char* buffer){
  buffer[0] = 0;
  buffer[1] = 5;
  *((uint16_t*)(buffer+2)) = htons((uint16_t) error_code);
  buffer += 4;
  strcpy(buffer, error_msg);
}


int tftp_msg_unpack_error(char* buffer, int buffer_len, int* error_code, char* error_msg){
  if (tftp_msg_type(buffer) != TFTP_TYPE_ERROR){
      LOG(LOG_ERR, "Expected ERROR message (5), found %d", tftp_msg_type(buffer));
      return 1;
    }

    *error_code = (int) ntohs(*((uint16_t*)(buffer+2)));
    if (*error_code < 0 || *error_code > 7){
      LOG(LOG_ERR, "Unrecognized error code: %d", *error_code);
      return 4;
    }

    buffer += 4;
    if(strlen(buffer) > TFTP_MAX_ERROR_LEN){
      LOG(LOG_ERR, "Error string too long (%d > %d): %s", (int) strlen(buffer), TFTP_MAX_ERROR_LEN, buffer);
      return 3;
    }

    strcpy(error_msg, buffer);
    if (buffer_len != strlen(error_msg)+5){
      LOG(LOG_WARN, "Packet contains unexpected fields");
      return 2;
    }
    return 0;
}


int tftp_msg_get_size_error(char* error_msg){
  return 5 + strlen(error_msg);
}
