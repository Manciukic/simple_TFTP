/**
 * @file
 * @author Riccardo Mancini
 * @brief Implementation of the TFTP server that can only handle read requests.
 * 
 * The server is multiprocessed, with each process handling one request.
 */


/** Defines log level to this file. */
#define LOG_LEVEL LOG_INFO


#include "include/tftp_msgs.h"
#include "include/tftp.h"
#include "include/fblock.h"
#include "include/inet_utils.h"
#include "include/debug_utils.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/logging.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

/** Maximum length for a RRQ message */
#define MAX_MSG_LEN TFTP_MAX_MODE_LEN+TFTP_MAX_FILENAME_LEN+4

/**
 * Prints command usage information.
 */
void print_help(){
  printf("Usage: ./tftp_server LISTEN_PORT FILES_DIR\n");
  printf("Example: ./tftp_server 69 .\n");
}

/**
 * Sends file to a client.
 */
int send_file(char* filename, char* mode, struct sockaddr_in *cl_addr){
  struct sockaddr_in my_addr;
  int sd;
  int ret, tid;
  struct fblock m_fblock;

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  my_addr = make_my_sockaddr_in(0);
  tid = bind_random_port(sd, &my_addr);
  if (tid == 0){
    LOG(LOG_ERR, "Could not bind to random port");
    perror("Could not bind to random port:");
    fblock_close(&m_fblock);
    return 2;
  } else
    LOG(LOG_INFO, "Bound to port %d", tid);

  if (strcmp(mode, TFTP_STR_OCTET) == 0){
    m_fblock = fblock_open(filename, TFTP_DATA_BLOCK, FBLOCK_READ|FBLOCK_MODE_BINARY);
  } else if (strcmp(mode, TFTP_STR_NETASCII) == 0){
    m_fblock = fblock_open(filename, TFTP_DATA_BLOCK, FBLOCK_READ|FBLOCK_MODE_TEXT);
  } else{
    LOG(LOG_ERR, "Unknown mode: %s", mode);
    return 2;
  }
  
  if (m_fblock.file == NULL){
    LOG(LOG_WARN, "Error opening file. Not found?");
    tftp_send_error(1, "File not found.", sd, cl_addr);
    return 1;
  }

  LOG(LOG_INFO, "Sending file...");

  ret = tftp_send_file(&m_fblock, sd, cl_addr);
  fblock_close(&m_fblock);

  if (ret != 0){
    LOG(LOG_ERR, "Error sending file: %d", ret);
    return 16+ret;
  }

  LOG(LOG_INFO, "File sent successfully");
  return 0;
}

/** Main */
int main(int argc, char** argv){
  short int my_port;
  char *directory;
  char filename[TFTP_MAX_FILENAME_LEN], *path, mode[TFTP_MAX_MODE_LEN];
  int ret, type, len;
  char in_buffer[MAX_MSG_LEN];
  unsigned int addrlen;
  int sd;
  struct sockaddr_in my_addr, cl_addr;
  int pid;

  if (argc != 3){
    print_help();
    return 1;
  }

  my_port = atoi(argv[1]);
  directory = argv[2];

  addrlen = sizeof(cl_addr);

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  my_addr = make_my_sockaddr_in(my_port);
  ret = bind(sd, (struct sockaddr*) &my_addr, sizeof(my_addr));
  if (ret == -1){
    LOG(LOG_ERR, "Could not bind");
    return 1;
  }

  LOG(LOG_INFO, "Server is running");

  while (1){
    len = recvfrom(sd, in_buffer, MAX_MSG_LEN, 0, (struct sockaddr*)&cl_addr, &addrlen);
    type = tftp_msg_type(in_buffer);
    LOG(LOG_DEBUG, "Received message with type %d", type);
    if (type == TFTP_TYPE_RRQ){
      pid = fork();
      if (pid != 0){
        LOG(LOG_INFO, "Received RRQ, spawned new process %d", (int) pid);
        continue;
      }

      //init random seed
      srand(time(NULL));

      ret = tftp_msg_unpack_rrq(in_buffer, len, filename, mode);

      path = malloc(strlen(filename)+strlen(directory)+2);
      path[0] = '\0';

      strcat(path, directory);
      strcat(path, "/");  // TODO: handle trailing / 
      strcat(path, filename);

      LOG(LOG_INFO, "User wants to read file %s in mode %s", filename, mode);

      ret = send_file(path, mode, &cl_addr);
      if (ret != 0)
        LOG(LOG_WARN, "Write terminated with an error: %d", ret);
      break;  // child process exits loop
    } else{
      LOG(LOG_WARN, "Wrong op code: %d", type);
      tftp_send_error(4, "Illegal TFTP operation.", sd, &cl_addr);
    } // father process continues loop
  }

  LOG(LOG_INFO, "Exiting process %d", (int) getpid());
  return 0;
}
