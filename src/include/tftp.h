#ifndef TFTP
#define TFTP

#include <sys/socket.h>
#include <netinet/in.h>
#include "fblock.h"

int tftp_send_rrq(char* filename, char *mode, int sd, struct sockaddr_in *addr);

int tftp_send_wrq(char* filename, char *mode, int sd, struct sockaddr_in *addr);

int tftp_send_error(int error_code, char* error_msg, int sd, struct sockaddr_in *addr);

int tftp_receive_file(struct fblock *m_fblock, int sd, struct sockaddr_in *addr);

int tftp_send_file(struct fblock *m_fblock, int sd, struct sockaddr_in *addr);

#endif
