/**
 * @file
 * @author Riccardo Mancini
 * 
 * @brief Utility functions for debugging.
 *
 * At the moment, this library implements only one function for dumping a 
 * buffer using hexadecimal.
 */

#ifndef DEBUG_UTILS
#define DEBUG_UTILS


/**
 * Prints content of buffer to stdout, showing it as hex values.
 * 
 * @param buffer    pointer to the buffer to be printed
 * @param len       the length (in bytes) of the buffer
 */
void dump_buffer_hex(char* buffer, int len);


#endif
