/**
 * @file
 * @author Riccardo Mancini
 * @brief Common functions for TFTP client and server
 *
 * This library provides functions for sending requests, errors and exchanging
 * files using the TFTP protocol.
 * 
 * Even though the project assignment does not require the client to send files 
 * to the server, I still decided to include those functions in a common library
 * in case in the future I decide to complete the TFTP implementation.
 */

#ifndef TFTP
#define TFTP


#include <sys/socket.h>
#include <netinet/in.h>
#include "fblock.h"


/**
 * Send a RRQ message to a server.
 * 
 * @param filename  the name of the requested file
 * @param mode      the desired mode of transfer (netascii or octet)
 * @param sd        socket id of the (UDP) socket to be used to send the message
 * @param addr      address of the server
 * @return          0 in case of success, 1 otherwise
 * 
 * @see TFTP_STR_NETASCII 
 * @see TFTP_STR_OCTET 
 */
int tftp_send_rrq(char* filename, char *mode, int sd, struct sockaddr_in *addr);

/**
 * Send a WRQ message to a server.
 * 
 * Do not used in current implementation.
 * 
 * @param filename  the name of the requested file
 * @param mode      the desired mode of transfer (netascii or octet)
 * @param sd        socket id of the (UDP) socket to be used to send the message
 * @param addr      address of the server
 * @return          0 in case of success, 1 otherwise 
 * 
 * @see TFTP_STR_NETASCII 
 * @see TFTP_STR_OCTET 
 */
int tftp_send_wrq(char* filename, char *mode, int sd, struct sockaddr_in *addr);

/**
 * Send an ERROR message to the client (server).
 * 
 * In current implementation it is only used for sending File Not Found and 
 * Illegal TFTP Operation errors to clients.
 * 
 * @param error_code the code of the error (must be within 0 and 7)
 * @param error_msg  the message explaining the error
 * @param sd         socket id of the (UDP) socket to be used to send the message
 * @param addr       address of the client (server)
 * @return           0 in case of success, 1 otherwise
 */
int tftp_send_error(int error_code, char* error_msg, int sd, struct sockaddr_in *addr);

/**
 * Send an ACK message.
 * 
 * In current implementation it is only used for sending ACKs from client to
 * server.
 * 
 * @param block_n    sequence number of the block to be acknowledged.
 * @param out_buffer buffer to be used for sending the ACK (useful for recycling the same buffer)
 * @param sd         socket id of the (UDP) socket to be used to send the message
 * @param addr       address of recipient of the ACK
 * @return           0 in case of success, 1 otherwise
 */
int tftp_send_ack(int block_n, char* out_buffer, int sd, struct sockaddr_in *addr);

/**
 * Handle the entire workflow required to receive a file.
 * 
 * In current implementation it is only used in client but it could be also 
 * used on the server side, potentially (some tweaks may be needed, though!).
 * 
 * @param m_fblock   block file where to write incoming data to
 * @param sd         socket id of the (UDP) socket to be used to send ACK messages
 * @param addr       address of the recipient of ACKs
 * @return
 * - 0 in case of success.
 * - 1 in case of file not found.
 * - 2 in case of error while sending ACK.
 * - 3 in case of unexpected sequence number.
 * - 4 in case of an error while unpacking data.
 * - 5 in case of an error while unpacking an incoming error message.
 * - 6 in case of en error while writing to the file.
 * - 7 in case of an error message different from File Not Found (since it is the only erorr available in current implementation).
 * - 8 in case of the incoming message is neither DATA nor ERROR.
 * - 9 in case of message from unexpected source
 */
int tftp_receive_file(struct fblock *m_fblock, int sd, struct sockaddr_in *addr);

/**
 * Receive an ACK message.
 * 
 * In current implementation it is only used for receiving ACKs from client.
 * 
 * @param block_n [out] sequence number of the acknowledged block.
 * @param in_buffer     buffer to be used for receiving the ACK (useful for recycling the same buffer)
 * @param sd [in]       socket id of the (UDP) socket to be used to send the message
 * @param addr [in]     address of recipient of the ACK
 * @return
 * - 0 in case of success
 * - 1 in case of failure while receiving the message
 * - 2 in case of address and/or port mismatch in sender sockaddr
 * - error unpacking ACK message otherwise (8 + result of tftp_msg_unpack_ack)
 * 
 * @see tftp_msg_unpack_ack
 */
int tftp_receive_ack(int *block_n, char* in_buffer, int sd, struct sockaddr_in *addr);

/**
 * Handle the entire workflow required to send a file.
 * 
 * In current implementation it is only used in server but it could be also 
 * used on the client side, potentially (some tweaks may be needed, though!).
 * 
 * @param m_fblock   block file where to read incoming data from
 * @param sd         socket id of the (UDP) socket to be used to send DATA messages
 * @param addr       address of the recipient of the file 
 * @return
 * - 0 in case of success.
 * - 1 in case of error sending a packet.
 * - 2 in case of error while receiving the ack.
 * - 3 in case of unexpected sequence number in ack.
 * - 4 in case of an error while unpacking data.
 */
int tftp_send_file(struct fblock *m_fblock, int sd, struct sockaddr_in *addr);


#endif
