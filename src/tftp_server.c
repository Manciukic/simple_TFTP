/**
 * @file
 * @author Riccardo Mancini
 * @brief Implementation of the TFTP server serving only read requests.
 */

#include "tftp_msgs.h"
#include "tftp.h"
#include "fblock.h"
#include "inet_utils.h"
#include "debug_utils.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FILENAME "prova.txt";
#define MODE     "netascii";

void print_help(){
  printf("Usage: ./tftp_server LISTEN_PORT FILES_DIR\n");
  printf("Example: ./tftp_server 69 .");
}


int write(char* filename, char* mode, struct sockaddr_in *cl_addr){
  struct sockaddr_in my_addr;
  int sd;
  int ret, tid;
  struct fblock m_fblock;

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (strcmp(mode, TFTP_STR_OCTET)){
    m_fblock = fblock_open(filename, TFTP_DATA_BLOCK, FBLOCK_READ|FBLOCK_MODE_BINARY);
  } else if (strcmp(mode, TFTP_STR_NETASCII)){
    m_fblock = fblock_open(filename, TFTP_DATA_BLOCK, FBLOCK_READ|FBLOCK_MODE_TEXT);
  } else{
    return 2;
  }
  if (m_fblock.file == NULL)
    return 1;

  my_addr = make_my_sockaddr_in(0);
  tid = bind_random_port(sd, &my_addr);
  if (tid == 0){
    perror("Could not bind to random port:");
    fblock_close(&m_fblock);
    return 2;
  } else
    printf("Bound to port %d\n", tid);

  ret = tftp_send_file(&m_fblock, sd, cl_addr);
  fblock_close(&m_fblock);
  if (ret!=0)
    return 16+ret;

  return 0;
}

int main(int argc, char** argv){
  short int my_port;
  char *directory, *filename, *path, *mode;
  int ret, max_msglen, type, len;
  char* in_buffer;
  unsigned int addrlen;
  int sd;
  struct sockaddr_in my_addr, cl_addr;

  if (argc != 3){
    print_help();
    return 1;
  }

  my_port = atoi(argv[1]);
  directory = argv[2];

  max_msglen = TFTP_MAX_MODE_LEN+TFTP_MAX_FILENAME_LEN+4;
  in_buffer = malloc(max_msglen);
  addrlen = sizeof(cl_addr);

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  my_addr = make_my_sockaddr_in(my_port);
  ret = bind(sd, (struct sockaddr*) &my_addr, sizeof(my_addr));
  if (ret == -1){
    printf("Could not bind\n");
    return 1;
  }

  while (1){
    len = recvfrom(sd, in_buffer, max_msglen, 0, (struct sockaddr*)&cl_addr, &addrlen);
    type = tftp_msg_type(in_buffer);
    printf("Received type %d.\n", type);
    if (type == TFTP_TYPE_RRQ){
      filename = malloc(TFTP_MAX_FILENAME_LEN);
      mode = malloc(TFTP_MAX_MODE_LEN);
      path = malloc(TFTP_MAX_FILENAME_LEN+strlen(directory));
      ret = tftp_msg_unpack_rrq(in_buffer, len, filename, mode);
      path[0] = '\0';
      strcat(path, directory);
      strcat(path, filename);
      ret = write(path, mode, &cl_addr);
      if (ret != 0)
        printf("Write terminated with an error: %d", ret);
    } else{
      printf("Wrong op code\n");
    }
  }

  return 0;
}
