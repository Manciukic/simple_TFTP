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


/** Read request message type */
#define TFTP_TYPE_RRQ   1

/** Write request message type */
#define TFTP_TYPE_WRQ   2

/** Data message type */
#define TFTP_TYPE_DATA  3

/** Acknowledgment message type */
#define TFTP_TYPE_ACK   4

/** Error message type */
#define TFTP_TYPE_ERROR 5

/** String for netascii */
#define TFTP_STR_NETASCII "netascii"

/** String for octet */
#define TFTP_STR_OCTET "octet"

/** Maximum filename length (do not defined in RFC) */
#define TFTP_MAX_FILENAME_LEN 255

/** 
 * Maximum mode field string length
 * 
 * Since there are only two options: 'netascii' and 'octet', len('netascii') is
 * the TFTP_MAX_MODE_LEN.
 */
#define TFTP_MAX_MODE_LEN 8

/** Maximum error message length (do not defined in RFC) */
#define TFTP_MAX_ERROR_LEN 255

/** Data block size as defined in RFC */
#define TFTP_DATA_BLOCK 512


/**
 * Retuns msg type given a message buffer.
 *
 * @param buffer the buffer
 * @return message type
 * 
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
 * ```
 *  2 bytes    string    1 byte    string    1 byte
 *  -----------------------------------------------
 * |   01  |  Filename  |   0  |    Mode    |   0  |
 *  -----------------------------------------------
 * ```
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
 * @return
 * - 0 in case of success.
 * - 1 in case of wrong operation code.
 * - 2 in case of unexpected fields inside message.
 * - 3 in case of filename exceeding TFTP_MAX_FILENAME_LEN.
 * - 4 in case of mode string exceeding TFTP_MAX_MODE_LEN.
 * - 5 in case of unrecognized transfer mode.
 * 
 * @see TFTP_TYPE_RRQ
 * @see TFTP_MAX_FILENAME_LEN
 * @see TFTP_MAX_MODE_LEN
 * @see TFTP_STR_NETASCII
 * @see TFTP_STR_OCTET
 */
int tftp_msg_unpack_rrq(char* buffer, int buffer_len, char* filename, char* mode);

/**
 * Returns size in bytes of a read request message.
 *
 * @param filename  name of the file
 * @param mode      requested transfer mode ("netascii" or "octet")
 * @return          size in bytes
 */
int tftp_msg_get_size_rrq(char* filename, char* mode);

/**
 * Builds a write request message.
 *
 * Message format:
 * ```
 *  2 bytes    string    1 byte    string    1 byte
 *  -----------------------------------------------
 * |   02  |  Filename  |   0  |    Mode    |   0  |
 *  -----------------------------------------------
 * ```
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
 * @return
 * - 0 in case of success.
 * - 1 in case of wrong operation code.
 * - 2 in case of unexpected fields inside message.
 * - 3 in case of filename exceeding TFTP_MAX_FILENAME_LEN.
 * - 4 in case of mode string exceeding TFTP_MAX_MODE_LEN.
 * - 5 in case of unrecognized transfer mode.
 * 
 * @see TFTP_TYPE_WRQ
 * @see TFTP_MAX_FILENAME_LEN
 * @see TFTP_MAX_MODE_LEN
 * @see TFTP_STR_NETASCII
 * @see TFTP_STR_OCTET
 */
int tftp_msg_unpack_rrq(char* buffer, int buffer_len, char* filename, char* mode);

/**
 * Returns size in bytes of a write request message.
 *
 * @param filename  name of the file
 * @param mode      requested transfer mode ("netascii" or "octet")
 * @return          size in bytes
 */
int tftp_msg_get_size_wrq(char* filename, char* mode);

/**
 * Builds a data message.
 *
 * Message format:
 * ```
 *  2 bytes    2 bytes       n bytes
 *  ---------------------------------
 * | 03    |   Block #  |    Data    |
 *  ---------------------------------
 * ```
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
 * @return
 * - 0 in case of success.
 * - 1 in case of wrong operation code.
 * - 2 in case of missing fields (packet size is too small).
 * 
 * @see TFTP_TYPE_DATA
 */
int tftp_msg_unpack_data(char* buffer, int buffer_len, int* block_n, char* data, int* data_size);

/**
 * Returns size in bytes of a data message.
 *
 * It just sums 4 to data_size.
 *
 * @param data_size data buffer size
 * @return          size in bytes
 */
int tftp_msg_get_size_data(int data_size);

/**
 * Builds an acknowledgment message.
 *
 * Message format:
 * ```
 *  2 bytes    2 bytes
 *  -------------------
 * | 04    |   Block #  |
 *  --------------------
 * ```
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
 * @return
 * - 0 in case of success.
 * - 1 in case of wrong operation code.
 * - 2 in case of wrong packet size.
 * 
 * @see TFTP_TYPE_ACK
 */
int tftp_msg_unpack_ack(char* buffer, int buffer_len, int* block_n);

/**
 * Returns size in bytes of an acknowledgment message.
 *
 * It just returns 4.
 *
 * @param data_size data buffer size
 * @return          size in bytes
 */
int tftp_msg_get_size_ack();

/**
 * Builds an error message.
 * 
 * Message format:
 * ```
 *   2 bytes  2 bytes        string    1 byte
 *  ----------------------------------------
 * | 05    |  ErrorCode |   ErrMsg   |   0  |
 *  ----------------------------------------
 * ```
 * 
 * Error code meaning:
 * - 0: Not defined, see error message (if any).
 * - 1: File not found.
 * - 2: Access violation.
 * - 3: Disk full or allocation exceeded.
 * - 4: Illegal TFTP operation.
 * - 5: Unknown transfer ID.
 * - 6: File already exists.
 * - 7: No such user.
 * 
 * In current implementation only errors 1 and 4 are implemented.
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
 * @return
 * - 0 in case of success.
 * - 1 in case of wrong operation code.
 * - 2 in case of unexpected fields.
 * - 3 in case of error string exceeding TFTP_MAX_ERROR_LEN.
 * - 4 in case of unrecognize error code (must be within 0 and 7).
 * 
 * @see TFTP_TYPE_ERROR
 * @see TFTP_MAX_ERROR_LEN
 */
int tftp_msg_unpack_error(char* buffer, int buffer_len, int* error_code, char* error_msg);

/**
 * Returns size in bytes of an error message.
 *
 * @param error_msg  error message
 * @return           size in bytes
 */
int tftp_msg_get_size_error(char* error_msg);


#endif
