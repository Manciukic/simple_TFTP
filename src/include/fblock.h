/**
 * @file
 * @author Riccardo Mancini
 * @brief File block read and write.
 *
 * This library provides functions for reading and writing a text or binary
 * file using a predefined block size.
 */

#ifndef FBLOCK
#define FBLOCK


#include <stdio.h>


/** Mask for getting text/binary mode */
#define FBLOCK_MODE_MASK   0b01

/** Open file in text mode */
#define FBLOCK_MODE_TEXT   0b00

/** Open file in binary mode */
#define FBLOCK_MODE_BINARY 0b01

/** Mask for getting r/w mode */
#define FBLOCK_RW_MASK     0b10

/** Open file in read mode */
#define FBLOCK_READ        0b00

/** Open file in write mode */
#define FBLOCK_WRITE       0b10


/**
 * Structure which defines a file.
 */
struct fblock{
  FILE *file; /**< Pointer to the file */
  int block_size;  /**< Predefined block size for i/o operations */
  char mode;  /**< Can be read xor write, text xor binary. */
  union{
    int written;  /**< Bytes already written (for future use) */
    int remaining;  /**< Remaining bytes to read  */
  };
};


/**
 * Opens a file.
 *
 * @param filename    name of the file
 * @param block_size  size of the blocks
 * @param mode        mode (read, write, text, binary)
 * @return            fblock structure
 * 
 * @see FBLOCK_MODE_TEXT
 * @see FBLOCK_MODE_BINARY
 * @see FBLOCK_WRITE
 * @see FBLOCK_READ
 */
struct fblock fblock_open(char* filename, int block_size, char mode);

/**
 * Reads next block_size bytes from file.
 *
 * @param m_fblock    fblock instance
 * @param buffer      block_size bytes buffer
 * @return            0 in case of success, otherwise number of bytes it could not read
 */
int fblock_read(struct fblock *m_fblock, char* buffer);

/**
 * Writes next block_size bytes to file.
 *
 * @param m_fblock    fblock instance
 * @param buffer      block_size bytes buffer
 * @param block_size  if set to a non-0 value, override block_size defined in fblock.
 * @return            0 in case of success, otherwise number of bytes it could not write
 */
int fblock_write(struct fblock *m_fblock, char* buffer, int block_size);

/**
 * Closes a file.
 *
 * @param m_fblock    fblock instance to be closed
 * @return            0 in case of success, EOF in case of failure
 * 
 * @see fclose
 */
int fblock_close(struct fblock *m_fblock);


#endif
