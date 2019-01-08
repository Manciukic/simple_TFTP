/**
 * @file
 * @author Riccardo Mancini
 * 
 * @brief Implementation of tftp.h.
 * 
 * @see tftp.h
 */


#include "include/fblock.h"
#include "include/tftp_msgs.h"
#include "include/debug_utils.h"
#include "include/inet_utils.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "include/logging.h"


/** LOG_LEVEL will be defined in another file */
extern const int LOG_LEVEL;


int tftp_send_rrq(char* filename, char *mode, int sd, struct sockaddr_in *addr){
  int msglen, len;
  char *out_buffer;

  msglen = tftp_msg_get_size_rrq(filename, mode);
  out_buffer = malloc(msglen);

  tftp_msg_build_rrq(filename, mode, out_buffer);
  len = sendto(sd, out_buffer, msglen, 0, (struct sockaddr*) addr, sizeof(*addr));
  if (len != msglen){
    LOG(LOG_ERR, "Error sending RRQ: len (%d) != msglen (%d)", len, msglen);
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
    LOG(LOG_ERR, "Error sending WRQ: len (%d) != msglen (%d)", len, msglen);
    perror("Error");
    return 1;
  }

  free(out_buffer);
  return 0;
}


int tftp_send_error(int error_code, char* error_msg, int sd, struct sockaddr_in *addr){
  int msglen, len;
  char *out_buffer;

  msglen = tftp_msg_get_size_error(error_msg);
  out_buffer = malloc(msglen);

  tftp_msg_build_error(error_code, error_msg, out_buffer);
  len = sendto(sd, out_buffer, msglen, 0, (struct sockaddr*) addr, sizeof(*addr));
  if (len != msglen){
    LOG(LOG_ERR, "Error sending ERROR: len (%d) != msglen (%d)", len, msglen);
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

 if (len != msglen){
    LOG(LOG_ERR, "Error sending ACK: len (%d) != msglen (%d)", len, msglen);
    perror("Error");
    return 1;
  }

  return 0;
}


int tftp_receive_file(struct fblock *m_fblock, int sd, struct sockaddr_in *addr){
  char in_buffer[TFTP_MAX_DATA_MSG_SIZE], data[TFTP_DATA_BLOCK], out_buffer[4];
  int exp_block_n, rcv_block_n;
  int len, data_size, ret, type;
  unsigned int addrlen;
  struct sockaddr_in cl_addr, orig_cl_addr;

  // init expected block number
  exp_block_n = 1;

  addrlen = sizeof(cl_addr);

  do{
    LOG(LOG_DEBUG, "Waiting for part %d", exp_block_n);
    // TODO: check client == server ?
    len = recvfrom(sd, in_buffer, tftp_msg_get_size_data(TFTP_DATA_BLOCK), 0, (struct sockaddr*)&cl_addr, &addrlen);
    if (exp_block_n == 1){ // first block -> I need to save servers TID (aka its "original" sockaddr)
      char addr_str[MAX_SOCKADDR_STR_LEN];
      sockaddr_in_to_string(cl_addr, addr_str); 
    
      if (addr->sin_addr.s_addr != cl_addr.sin_addr.s_addr){
        LOG(LOG_WARN, "Received message from unexpected source: %s", addr_str);
        continue;
      } else{
        LOG(LOG_INFO, "Receiving packets from %s", addr_str);
        orig_cl_addr = cl_addr;
      }
    } else{
      if (sockaddr_in_cmp(orig_cl_addr, cl_addr) != 0){
        char addr_str[MAX_SOCKADDR_STR_LEN];
        sockaddr_in_to_string(cl_addr, addr_str); 
        LOG(LOG_WARN, "Received message from unexpected source: %s", addr_str);
        continue;
      } else{
        LOG(LOG_DEBUG, "Sender is the same!");
      }
    }
    
    type = tftp_msg_type(in_buffer);
    if (type == TFTP_TYPE_ERROR){
      int error_code;
      char error_msg[TFTP_MAX_ERROR_LEN];
      
      ret = tftp_msg_unpack_error(in_buffer, len, &error_code, error_msg);
      if (ret != 0){
        LOG(LOG_ERR, "Error unpacking error msg");
        return 5;
      }

      if (error_code == 1){
        LOG(LOG_INFO, "File not found");
        return 1;
      } else{
        LOG(LOG_ERR, "Received error %d: %s", error_code, error_msg);
        return 7;
      }

    } else if (type != TFTP_TYPE_DATA){
      LOG(LOG_ERR, "Received packet of type %d, expecting DATA or ERROR.", type);
      return 8;
    }

    ret = tftp_msg_unpack_data(in_buffer, len, &rcv_block_n, data, &data_size);

    if (ret != 0){
      LOG(LOG_ERR, "Error unpacking data: %d", ret);
      return 4;
    }

    if (rcv_block_n != exp_block_n){
      LOG(LOG_ERR, "Received unexpected block_n: rcv_block_n = %d != %d = exp_block_n", rcv_block_n, exp_block_n);
      return 3;
    }

    exp_block_n++;

    LOG(LOG_DEBUG, "Part %d has size %d", rcv_block_n, data_size);

    if (data_size != 0){
      if (fblock_write(m_fblock, data, data_size))
        return 6;
    }
    
    LOG(LOG_DEBUG, "Sending ack");

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

  if (sockaddr_in_cmp(*addr, cl_addr) != 0){
    char str_addr[MAX_SOCKADDR_STR_LEN];
    sockaddr_in_to_string(cl_addr, str_addr);
    LOG(LOG_WARN, "Message is coming from unexpected source: %s", str_addr);
    return 2;
  }

  if (len != msglen){
    LOG(LOG_ERR, "Error receiving ACK: len (%d) != msglen (%d)", len, msglen);
    return 1;
  }

  ret = tftp_msg_unpack_ack(in_buffer, len, block_n);
  if (ret != 0){
    LOG(LOG_ERR, "Error unpacking ack: %d", ret);
    return 8+ret;
  }

  return 0;
}


int tftp_send_file(struct fblock *m_fblock, int sd, struct sockaddr_in *addr){
  char in_buffer[4], data[TFTP_DATA_BLOCK], out_buffer[TFTP_MAX_DATA_MSG_SIZE];
  int block_n, rcv_block_n;
  int len, data_size, msglen, ret;

  // init sequence number
  block_n = 1;

  do{
    LOG(LOG_DEBUG, "Sending part %d", block_n);

    if (m_fblock->remaining > TFTP_DATA_BLOCK)
      data_size = TFTP_DATA_BLOCK;
    else
      data_size = m_fblock->remaining;

    if (data_size != 0)
      fblock_read(m_fblock, data);

    LOG(LOG_DEBUG, "Part %d has size %d", block_n, data_size);

    msglen = tftp_msg_get_size_data(data_size);
    tftp_msg_build_data(block_n, data, data_size, out_buffer);
    
    // dump_buffer_hex(out_buffer, msglen);
    
    len = sendto(sd, out_buffer, msglen, 0, (struct sockaddr*)addr, sizeof(*addr));

    if (len != msglen){
      return 1;
    }

    LOG(LOG_DEBUG, "Waiting for ack");

    ret = tftp_receive_ack(&rcv_block_n, in_buffer, sd, addr);

    if (ret == 2){  //unexpected source
      continue;
    } else if (ret != 0){
      LOG(LOG_ERR, "Error receiving ack");
      return 2;
    }

    if (rcv_block_n != block_n){
      LOG(LOG_ERR, "Received wrong block n: received %d != expected %d", rcv_block_n, block_n);
      return 3;
    }

    block_n++;

  } while(data_size == TFTP_DATA_BLOCK);
  return 0;
}
