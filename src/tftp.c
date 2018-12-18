#include "fblock.h"
#include "tftp_msgs.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>


int tftp_send_rrq(char* filename, char *mode, int sd, struct sockaddr_in *addr){
  int msglen, len;
  char *out_buffer;

  msglen = tftp_msg_get_size_rrq(filename, mode);
  out_buffer = malloc(msglen);

  tftp_msg_build_rrq(filename, mode, out_buffer);
  len = sendto(sd, out_buffer, msglen, 0, (struct sockaddr*) addr, sizeof(*addr));
  if (len != msglen){
    perror("Error");
    return 1;
  }

  free(out_buffer);
  return 0;
}

int tftp_send_wrq(char* filename, char *mode, int sd, struct sockaddr_in *addr){
  int msglen, len;
  char *out_buffer;

  msglen = tftp_msg_get_size_wrq(filename, mode);
  out_buffer = malloc(msglen);

  tftp_msg_build_wrq(filename, mode, out_buffer);
  len = sendto(sd, out_buffer, msglen, 0, (struct sockaddr*) addr, sizeof(*addr));
  if (len != msglen){
    perror("Error");
    return 1;
  }

  free(out_buffer);
  return 0;
}

int tftp_send_ack(int block_n, char* out_buffer, int sd, struct sockaddr_in *addr){
  int msglen, len;

  msglen = tftp_msg_get_size_ack();
  tftp_msg_build_ack(block_n, out_buffer);
  len = sendto(sd, out_buffer, msglen, 0, (struct sockaddr*) addr, sizeof(*addr));

  return len - msglen;
}

int tftp_receive_file(struct fblock *m_fblock, int sd, struct sockaddr_in *addr){
  char *in_buffer, *data, *out_buffer;
  int exp_block_n, rcv_block_n;
  int len, data_size, ret;
  unsigned int addrlen;
  struct sockaddr_in cl_addr;

  in_buffer = malloc(tftp_msg_get_size_data(TFTP_DATA_BLOCK));
  data = malloc(TFTP_DATA_BLOCK);
  out_buffer = malloc(4);

  exp_block_n = 1;
  addrlen = sizeof(cl_addr);

  do{
    printf("Waiting for part %d\n", exp_block_n);
    // check client == server ?
    len = recvfrom(sd, in_buffer, tftp_msg_get_size_data(TFTP_DATA_BLOCK), 0, (struct sockaddr*)&cl_addr, &addrlen);
    ret = tftp_msg_unpack_data(in_buffer, len, &rcv_block_n, data, &data_size);

    if (ret != 0){
      printf("Error unpacking data: %d\n", ret);
      return 4;
    }

    if (rcv_block_n != exp_block_n){
      printf("rcv_block_n = %d != %d = exp_block_n\n", rcv_block_n, exp_block_n);
      return 3;
    }
    exp_block_n++;

    if (fblock_write(m_fblock, data, data_size))
      return 1;

    printf("Sending ack\n");

    if (tftp_send_ack(rcv_block_n, out_buffer, sd, &cl_addr))
      return 2;

  } while(data_size == TFTP_DATA_BLOCK);
  return 0;
}

int tftp_receive_ack(int *block_n, char* in_buffer, int sd, struct sockaddr_in *addr){
  int msglen, len, ret;
  unsigned int addrlen;
  struct sockaddr_in cl_addr;

  msglen = tftp_msg_get_size_ack();
  addrlen = sizeof(cl_addr);

  len = recvfrom(sd, in_buffer, msglen, 0, (struct sockaddr*)&cl_addr, &addrlen);

  if (len != msglen)
    return 1;

  ret = tftp_msg_unpack_ack(in_buffer, len, block_n);
  if (ret != 0){
    printf("Error unpacking ack: %d\n", ret);
    return 8+ret;
  }

  return 0;
}

int tftp_send_file(struct fblock *m_fblock, int sd, struct sockaddr_in *addr){
  char *in_buffer, *data, *out_buffer;
  int block_n, rcv_block_n;
  int len, data_size, msglen;

  in_buffer = malloc(4);
  data = malloc(TFTP_DATA_BLOCK);
  out_buffer = malloc(tftp_msg_get_size_data(TFTP_DATA_BLOCK));

  block_n = 1;

  do{
    printf("Sending part %d\n", block_n);

    if (m_fblock->remaining > TFTP_DATA_BLOCK)
      data_size = TFTP_DATA_BLOCK;
    else
      data_size = m_fblock->remaining;

    if (data_size != 0)
      fblock_read(m_fblock, data);

    msglen = tftp_msg_get_size_data(data_size);
    tftp_msg_build_data(block_n, data, data_size, out_buffer);
    len = sendto(sd, out_buffer, msglen, 0, (struct sockaddr*)addr, sizeof(*addr));

    if (len != msglen){
      return 1;
    }

    printf("Waiting for ack\n");

    if (tftp_receive_ack(&rcv_block_n, in_buffer, sd, addr))
      //Error
      return 2;

    if (rcv_block_n != block_n){
      //Error
      return 3;
    }
    block_n++;

    //carico prossima parte mentre aspetto ack?
  } while(data_size == TFTP_DATA_BLOCK);
  return 0;
}
