/**
 * @file
 * @author Riccardo Mancini
 * 
 * @brief Implementation of fblock.h.
 * 
 * @see fblock.h
 */

/** Defines log level to this file. */
#define LOG_LEVEL LOG_INFO


#include "include/fblock.h"
#include <stdio.h>
#include <string.h>
#include "include/logging.h"


/**
 * Returns file length
 *
 * @param f  file pointer
 * @return   file length in bytes
 */
int get_length(FILE *f){
  int size;
  fseek(f, 0, SEEK_END); // seek to end of file
  size = ftell(f); // get current file pointer
  fseek(f, 0, SEEK_SET); // seek back to beginning of file
  return size;
}


struct fblock fblock_open(char* filename, int block_size, char mode){
  struct fblock m_fblock;
  m_fblock.block_size = block_size;
  m_fblock.mode = mode;

  char mode_str[4] = "";

  LOG(LOG_DEBUG, "Opening file %s (%s %s), block_size = %d", 
      filename, 
      (mode & FBLOCK_MODE_MASK) == FBLOCK_MODE_BINARY ? "binary" : "text",
      (mode & FBLOCK_RW_MASK) == FBLOCK_WRITE ? "write" : "read",
      block_size
  );

  if ((mode & FBLOCK_RW_MASK) == FBLOCK_WRITE){
    strcat(mode_str, "w");
    m_fblock.written = 0;
  } else {
    strcat(mode_str, "r");
  }

  if ((mode & FBLOCK_MODE_MASK) == FBLOCK_MODE_BINARY)
    strcat(mode_str, "b");
  // text otherwise

  m_fblock.file = fopen(filename, mode_str);
  if (m_fblock.file == NULL){
    LOG(LOG_ERR, "Error while opening file %s", filename);
    return m_fblock;
  }
  if ((mode & FBLOCK_RW_MASK) == FBLOCK_READ)
    m_fblock.remaining = get_length(m_fblock.file);

  LOG(LOG_DEBUG, "Successfully opened file");
  return m_fblock;
}


int fblock_read(struct fblock *m_fblock, char* buffer){
  int bytes_read, bytes_to_read;

  if (m_fblock->remaining > m_fblock->block_size)
    bytes_to_read = m_fblock->block_size;
  else
    bytes_to_read = m_fblock->remaining;

  bytes_read = fread(buffer, sizeof(char), bytes_to_read, m_fblock->file);
  m_fblock->remaining -= bytes_read;

  return bytes_to_read - bytes_read;
}


int fblock_write(struct fblock *m_fblock, char* buffer, int block_size){
  int written_bytes;

  if (!block_size)
    block_size = m_fblock->block_size;

  written_bytes = fwrite(buffer, sizeof(char), block_size, m_fblock->file);
  m_fblock->written += written_bytes;
  return block_size - written_bytes;
}

int fblock_close(struct fblock *m_fblock){
  return fclose(m_fblock->file);
}
