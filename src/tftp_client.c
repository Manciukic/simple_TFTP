/**
 * @file
 * @author Riccardo Mancini
 * @brief Implementation of the TFTP client making only read requests.
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
#define MODE     "octet";

void print_help(){
  printf("Usage: ./tftp_client SERVER_IP SERVER_PORT\n");
  printf("Example: ./tftp_client 127.0.0.1 69");
}

int read(char* filename, char* mode, char* sv_ip, int sv_port){
  struct sockaddr_in my_addr, sv_addr;
  int sd;
  int ret, tid;
  struct fblock m_fblock;

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (strcmp(mode, TFTP_STR_OCTET))
    m_fblock = fblock_open(filename, TFTP_DATA_BLOCK, FBLOCK_WRITE|FBLOCK_MODE_BINARY);
  else if (strcmp(mode, TFTP_STR_NETASCII))
    m_fblock = fblock_open(filename, TFTP_DATA_BLOCK, FBLOCK_WRITE|FBLOCK_MODE_TEXT);
  else
    return 2;

  sv_addr = make_sv_sockaddr_in(sv_ip, sv_port);
  my_addr = make_my_sockaddr_in(0);
  tid = bind_random_port(sd, &my_addr);
  if (tid == 0){
    perror("Could not bind to random port:");
    fblock_close(&m_fblock);
    return 1;
  } else
    printf("Bound to port %d\n", tid);

  ret = tftp_send_rrq(filename, mode, sd, &sv_addr);
  if (ret != 0){
    fblock_close(&m_fblock);
    return 8+ret;
  }

  ret = tftp_receive_file(&m_fblock, sd, &sv_addr);
  fblock_close(&m_fblock);
  if (ret!=0)
    return 16+ret;

  return 0;

}

int main(int argc, char** argv){
  char* sv_ip;
  short int sv_port;
  int ret;

  char *filename = FILENAME;
  char *mode = MODE;

  if (argc != 3){
    print_help();
    return 1;
  }

  sv_ip = argv[1];
  sv_port = atoi(argv[2]);

  ret = read(filename, mode, sv_ip, sv_port);
  printf("ret: %d\n", ret);
  return ret;
}
