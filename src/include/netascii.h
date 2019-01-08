/**
 * @file
 * @author Riccardo Mancini
 * 
 * @brief Conversion functions from netascii to Unix standard ASCII.
 *
 * This library provides two functions to convert a file from netascii to Unix
 * standard ASCII and viceversa.
 * In particular, there are only two differences:
 * - `LF` in Unix becomes `CRLF` in netascii
 * - `CR` in Unix becomes `CRNUL` in netascii
 * 
 * @see https://tools.ietf.org/html/rfc764
 */


#ifndef NETASCII
#define NETASCII


/**
 * Unix to netascii conversion.
 * 
 * @param unix_filename     the filename of the input Unix file
 * @param netascii_filename the filename of the output netascii file
 * @return 
 * - 0 in case of success
 */ 
int unix2netascii(char *unix_filename, char* netascii_filename);

/**
 * Netascii to Unix conversion.
 * 
 * @param netascii_filename the filename of the input netascii file
 * @param unix_filename     the filename of the output Unix file
 * @return 
 * - 0 in case of success
 */ 
int netascii2unix(char* netascii_filename, char *unix_filename);


#endif
