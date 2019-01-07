/**
 * @file
 * @author Riccardo Mancini
 * @brief Implementation of inet_utils.h.
 * 
 * @see inet_utils.h
 */


/** Defines log level to this file. */
#define LOG_LEVEL LOG_INFO


#include "include/inet_utils.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "include/logging.h"


int bind_random_port(int socket, struct sockaddr_in *addr){
  int port, ret, i;
  for (i=0; i<MAX_TRIES; i++){
    if (i == 0) // first I generate a random one
      port = rand() % (TO_PORT - FROM_PORT + 1) + FROM_PORT;
    else  //if it's not free I scan the next one
      port = (port-FROM_PORT+1) % (TO_PORT - FROM_PORT + 1) + FROM_PORT;

    LOG(LOG_DEBUG, "Trying port %d...", port);

    addr->sin_port = htons(port);
    ret = bind(socket, (struct sockaddr*) addr, sizeof(*addr));
    if (ret != -1)
      return port;
    // consider only some errors?
  }
  LOG(LOG_ERR, "Could not bind to random port after %d attempts", MAX_TRIES);
  return 0;
}


struct sockaddr_in make_sv_sockaddr_in(char* ip, int port){
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, ip, &addr.sin_addr);
  return addr;
}


struct sockaddr_in make_my_sockaddr_in(int port){
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  return addr;
}
