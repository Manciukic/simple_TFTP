/**
 * @file
 * @author Riccardo Mancini
 * @brief Utility funcions for managing inet addresses
 *
 * This library provides functions for creating sockaddr_in structures from
 * IP address string and integer port number and for binding to a random 
 * port (chosen using rand() builtin C function).
 * 
 * @see sockaddr_in
 * @see rand
 */

#ifndef INET_UTILS
#define INET_UTILS


#include <sys/socket.h>
#include <netinet/in.h>

/** Random port will be greater or equal to FROM_PORT */
#define FROM_PORT 49152

/** Random port will be lower or equal to TO_PORT */
#define TO_PORT   65535

/** Maximum number of trials before giving up opening a random port */
#define MAX_TRIES 256


/**
 * Binds socket to a random port.
 *
 * @param socket    socket ID
 * @param addr      inet addr structure
 * @return          0 in case of failure, port it could bind to otherwise
 * 
 * @see FROM_PORT
 * @see TO_PORT
 * @see MAX_TRIES
 */
int bind_random_port(int socket, struct sockaddr_in *addr);

/**
 * Makes sockaddr_in structure given ip string and port of server.
 *
 * @param ip      ip address of server
 * @param port    port of the server
 * @return        sockaddr_in structure for the given server
 */
struct sockaddr_in make_sv_sockaddr_in(char* ip, int port);

/**
 * Makes sockaddr_in structure of this host.
 *
 * INADDR_ANY is used as IP address.
 *
 * @param port    port of the server
 * @return        sockaddr_in structure this host on given port
 */
struct sockaddr_in make_my_sockaddr_in(int port);


#endif
