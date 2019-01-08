/**
 * @file
 * @author Riccardo Mancini
 * 
 * @brief Implementation of netascii.h.
 * 
 * @see netascii.h
 */


#include "include/netascii.h"
#include "include/logging.h"
#include <stdio.h>


int unix2netascii(char *unix_filename, char* netascii_filename){
  FILE *unixf, *netasciif;
  char prev, tmp;
  int ret, result;

  unixf = fopen(unix_filename, "r");

  if (unixf == NULL){
    LOG(LOG_ERR, "Error opening file %s", unix_filename);
    return 1;
  }

  netasciif = fopen(netascii_filename, "w");

  if (unixf == NULL){
    LOG(LOG_ERR, "Error opening file %s", netascii_filename);
    return 2;
  }

  prev = EOF;

  while ((tmp = (char) fgetc(unixf)) != EOF){
    if (tmp == '\n' && prev != '\r'){ // LF -> CRLF
      ret = putc('\r', netasciif);
      if (ret == EOF)
        break;

      ret = putc('\n', netasciif);
      if (ret == EOF)
        break;

    } else if (tmp == '\r'){  // CR -> CRNUL
      char next = (char) fgetc(unixf);
      if (next != '\0')
        ungetc(next, unixf);

      ret = putc('\r', netasciif);
      if (ret == EOF)
        break;

      ret = putc('\0', netasciif);
      if (ret == EOF)
        break;      
    } else{
      ret = putc(tmp, netasciif);
      if (ret == EOF)
        break;
    }

    prev = tmp;
  }

  // Error writing to netasciif
  if (ret == EOF){
    LOG(LOG_ERR, "Error writing to file %s", netascii_filename);
    result = 3;
  } else{
    LOG(LOG_INFO, "Unix file %s converted to netascii file %s", unix_filename, netascii_filename);
    result = 0;
  }

  fclose(unixf);
  fclose(netasciif);

  return result;
}

int netascii2unix(char* netascii_filename, char *unix_filename){
  FILE *unixf, *netasciif;
  char tmp;
  int ret;
  int result = 0;

  unixf = fopen(unix_filename, "w");

  if (unixf == NULL){
    LOG(LOG_ERR, "Error opening file %s", unix_filename);
    return 1;
  }

  netasciif = fopen(netascii_filename, "r");

  if (unixf == NULL){
    LOG(LOG_ERR, "Error opening file %s", netascii_filename);
    return 2;
  }

  while ((tmp = (char) fgetc(netasciif)) != EOF){
    if (tmp == '\r'){  // CRLF -> LF ; CRNUL -> CR
      char next = (char) fgetc(netasciif);
      if (next == '\0'){  // CRNUL -> CR
       ret = putc('\r', unixf);
       if (ret == EOF)
        break;
      } else if (next == '\n'){  // CRLF -> LF
       ret = putc('\n', unixf);
       if (ret == EOF)
        break;
      } else if (next == EOF) { // bad format
        LOG(LOG_ERR, "Bad formatted netascii: unexpected EOF after CR");
        result = 4;
        break;  
      } else{                   // bad format
        LOG(LOG_ERR, "Bad formatted netascii: unexpected 0x%x after CR", next);
        result = 4;
        break;
      } 
    } else{

      // nothing else needs to be done!

      ret = putc(tmp, unixf);
      if (ret == EOF)
        break;
    }
  }

  if (result == 0){
    // Error writing to unixf
    if (ret == EOF){
      LOG(LOG_ERR, "Error writing to file %s", unix_filename);
      result = 3;
    } else{
      LOG(LOG_INFO, "Netascii file %s converted to Unix file %s", netascii_filename, unix_filename);
      result = 0;
    }
  } // otherwise there was an error (4 or 5) and result was already set

  fclose(unixf);
  fclose(netasciif);

  return result;
}
