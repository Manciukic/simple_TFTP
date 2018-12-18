/**
 * @file
 * @author Riccardo Mancini
 * @brief Contructor for TFTP messages
 *
 * This library provides functions for building TFTP messages.
 * There are 5 types of messages:
 *  - 1: Read request (RRQ)
 *  - 2: Write request (WRQ)
 *  - 3: Data (DATA)
 *  - 4: Acknowledgment (ACK)
 *  - 5: Error (ERROR)
 */

#ifndef TFTP_MSGS
#define TFTP_MSGS

/** Read request */
#define TFTP_TYPE_RRQ   1
/** Write request */
#define TFTP_TYPE_WRQ   2
/** Data */
#define TFTP_TYPE_DATA  3
/** Acknowledgment */
#define TFTP_TYPE_ACK   4
/** Error */
#define TFTP_TYPE_ERROR 5

#define TFTP_STR_NETASCII "netascii"
#define TFTP_STR_OCTET "octet"

#define TFTP_MAX_FILENAME_LEN 255
#define TFTP_MAX_MODE_LEN 8
#define TFTP_MAX_ERROR_LEN 255

#define TFTP_DATA_BLOCK 512


/**
 * Retuns msg type given message buffer.
 *
 * @param buffer the buffer
 * @returns message type
 * @see TFTP_TYPE_RRQ
 * @see TFTP_TYPE_WRQ
 * @see TFTP_TYPE_DATA
 * @see TFTP_TYPE_ACK
 * @see TFTP_TYPE_ERROR
 */
int tftp_msg_type(char *buffer);


/**
 * Builds a read request message.
 *
 *  2 bytes    string    1 byte    string    1 byte
 *  -----------------------------------------------
 * |   01  |  Filename  |   0  |    Mode    |   0  |
 *  -----------------------------------------------
 *
 * @param filename  name of the file
 * @param mode      requested transfer mode ("netascii" or "octet")
 * @param buffer    data buffer where to build the message
 */
void tftp_msg_build_rrq(char* filename, char* mode, char* buffer);

/**
 * Unpacks a read request message.
 *
 * @param buffer      data buffer where the message to read is [in]
 * @param buffer_len  length of the buffer [in]
 * @param filename    name of the file [out]
 * @param mode        requested transfer mode ("netascii" or "octet") [out]
 * @returns          0 if success, 1 if wrong opcode, 2 otherwise
 */
int tftp_msg_unpack_rrq(char* buffer, int buffer_len, char* filename, char* mode);

/**
 * Returns size in bytes of a read request message.
 *
 * @param filename  name of the file
 * @param mode      requested transfer mode ("netascii" or "octet")
 * @returns         size in bytes
 */
int tftp_msg_get_size_rrq(char* filename, char* mode);

/**
 * Builds a write request message.
 *
 *  2 bytes    string    1 byte    string    1 byte
 *  -----------------------------------------------
 * |   02  |  Filename  |   0  |    Mode    |   0  |
 *  -----------------------------------------------
 *
 * @param filename  name of the file
 * @param mode      requested transfer mode ("netascii" or "octet")
 * @param buffer    data buffer where to build the message
 */
void tftp_msg_build_wrq(char* filename, char* mode, char* buffer);

/**
 * Unpacks a write request message.
 *
 * @param buffer      data buffer where the message to read is [in]
 * @param buffer_len  length of the buffer [in]
 * @param filename    name of the file [out]
 * @param mode        requested transfer mode ("netascii" or "octet") [out]
 * @returns          0 if success, 1 if wrong opcode, 2 otherwise
 */
int tftp_msg_unpack_rrq(char* buffer, int buffer_len, char* filename, char* mode);

/**
 * Returns size in bytes of a write request message.
 *
 * @param filename  name of the file
 * @param mode      requested transfer mode ("netascii" or "octet")
 * @returns         size in bytes
 */
int tftp_msg_get_size_wrq(char* filename, char* mode);

/**
 * Builds a data message.
 *
 *  2 bytes    2 bytes       n bytes
 *  ---------------------------------
 * | 03    |   Block #  |    Data    |
 *  ---------------------------------
 *
 * @param block_n   block sequence number
 * @param data      pointer to the buffer containing the data to be transfered
 * @param data_size data buffer size
 * @param buffer    data buffer where to build the message
 */
void tftp_msg_build_data(int block_n, char* data, int data_size, char* buffer);

/**
 * Unpacks a data message.
 *
 * @param buffer      data buffer where the message to read is [in]
 * @param buffer_len  length of the buffer [in]
 * @param block_n     pointer where block_n will be written [out]
 * @param data        pointer where to copy data [out]
 * @returns           0 if success, 1 if wrong opcode, 2 otherwise
 */
int tftp_msg_unpack_data(char* buffer, int buffer_len, int* block_n, char* data, int* data_size);

/**
 * Returns size in bytes of a data message.
 *
 * It just sums 4 to data_size.
 *
 * @param data_size data buffer size
 * @returns         size in bytes
 */
int tftp_msg_get_size_data(int data_size);

/**
 * Builds an acknowledgment message.
 *
 *  2 bytes    2 bytes
 *  -------------------
 * | 04    |   Block #  |
 *  --------------------
 *
 * @param block_n   block sequence number
 * @param buffer    data buffer where to build the message
 */
void tftp_msg_build_ack(int block_n, char* buffer);

/**
 * Unpacks an acknowledgment message.
 *
 * @param buffer     data buffer where the message to read is [in]
 * @param buffer_len length of the buffer [in]
 * @param block_n    pointer where block_n will be written [out]
 * @param data       pointer inside buffer where the data is [out]
 * @returns          0 if success, 1 if wrong opcode, 2 otherwise
 */
int tftp_msg_unpack_ack(char* buffer, int buffer_len, int* block_n);

/**
 * Returns size in bytes of an acknowledgment message.
 *
 * It just returns 4.
 *
 * @param data_size data buffer size
 * @returns         size in bytes
 */
int tftp_msg_get_size_ack();

/**
 * Builds an error message.
 *
 *   2 bytes  2 bytes        string    1 byte
 *  ----------------------------------------
 * | 05    |  ErrorCode |   ErrMsg   |   0  |
 *  ----------------------------------------
 *
 * @param error_code error code (from 0 to 7)
 * @param error_msg  error message
 * @param buffer    data buffer where to build the message
 */
void tftp_msg_build_error(int error_code, char* error_msg, char* buffer);

/**
 * Unpacks an error message.
 *
 * @param buffer     data buffer where the message to read is [in]
 * @param buffer_len length of the buffer [in]
 * @param error_code pointer where error_code will be written [out]
 * @param error_msg  pointer to error message inside the message [out]
 * @returns          0 if success, 1 if wrong opcode, 2 otherwise
 */
int tftp_msg_unpack_error(char* buffer, int buffer_len, int* error_code, char* error_msg);

/**
 * Returns size in bytes of an error message.
 *
 * @param error_msg  error message
 * @returns         size in bytes
 */
int tftp_msg_get_size_error(char* error_msg);

#endif
