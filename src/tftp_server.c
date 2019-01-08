/**
 * @file
 * @author Riccardo Mancini
 * @brief Implementation of the TFTP server that can only handle read requests.
 * 
 * The server is multiprocessed, with each process handling one request.
 */


/** Defines log level to this file. */
#define LOG_LEVEL LOG_DEBUG

#define _GNU_SOURCE
#include <stdlib.h>

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
#include "include/logging.h"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <linux/limits.h>
#include <libgen.h>


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
  char *dir_rel_path;
  char *ret_realpath;
  char dir_realpath[PATH_MAX];
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
  dir_rel_path = argv[2];

  ret_realpath = realpath(dir_rel_path, dir_realpath);
  if (ret_realpath == NULL){
    LOG(LOG_FATAL, "Directory not found: %s", dir_rel_path);
    return 1;
  }

  addrlen = sizeof(cl_addr);

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  my_addr = make_my_sockaddr_in(my_port);
  ret = bind(sd, (struct sockaddr*) &my_addr, sizeof(my_addr));
  if (ret == -1){
    perror("Could not bind: ");
    LOG(LOG_FATAL, "Could not bind to port %d", my_port);
    return 1;
  }

  LOG(LOG_INFO, "Server is running");

  while (1){
    len = recvfrom(sd, in_buffer, MAX_MSG_LEN, 0, (struct sockaddr*)&cl_addr, &addrlen);
    type = tftp_msg_type(in_buffer);
    LOG(LOG_DEBUG, "Received message with type %d", type);
    if (type == TFTP_TYPE_RRQ){
      pid = fork();
      if (pid != 0){  // father
        LOG(LOG_INFO, "Received RRQ, spawned new process %d", (int) pid);
        continue; // father process continues loop
      } else{         // child
        char filename[TFTP_MAX_FILENAME_LEN], mode[TFTP_MAX_MODE_LEN];
        char file_path[PATH_MAX], file_realpath[PATH_MAX];

        //init random seed
        srand(time(NULL));

        ret = tftp_msg_unpack_rrq(in_buffer, len, filename, mode);

        strcpy(file_path, dir_realpath);
        strcat(file_path, "/");
        strcat(file_path, filename);
        ret_realpath = realpath(file_path, file_realpath);

        if (ret_realpath == NULL){
          LOG(LOG_WARN, "File not found: %s", file_path);
          tftp_send_error(1, "File Not Found.", sd, &cl_addr);
          break; // child process exits loop
        }

        // check if file is inside directory (or inside any of its subdirectories)
        if (strncmp(file_realpath, dir_realpath, strlen(dir_realpath)) != 0){
          // it is not! I caught you, Trudy!
          LOG(LOG_WARN, "User tried to access file %s outside set directory %s", 
              file_realpath, 
              dir_realpath
          );

          tftp_send_error(4, "Access violation.", sd, &cl_addr);
          break; // child process exits loop
        }

        LOG(LOG_INFO, "User wants to read file %s in mode %s", filename, mode);

        ret = send_file(file_realpath, mode, &cl_addr);
        if (ret != 0)
          LOG(LOG_WARN, "Write terminated with an error: %d", ret);
        break;  // child process exits loop
      }
    } else{
      LOG(LOG_WARN, "Wrong op code: %d", type);
      tftp_send_error(4, "Illegal TFTP operation.", sd, &cl_addr);
      // main process continues loop
    } 
  }

  LOG(LOG_INFO, "Exiting process %d", (int) getpid());
  return 0;
}
