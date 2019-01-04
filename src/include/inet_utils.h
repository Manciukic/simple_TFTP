#ifndef INET_UTILS
#define INET_UTILS

#include <sys/socket.h>
#include <netinet/in.h>

#define FROM_PORT 49152
#define TO_PORT   65535
#define MAX_TRIES 256

/**
 * Binds socket to a random port.
 *
 * @param socket    socket ID
 * @param addr      inet addr
 * @returns         0 in case of failure, port it could bind to otherwise
 */
int bind_random_port(int socket, struct sockaddr_in *addr);

/**
 * Makes sockaddr_in structure given ip string and port of server.
 *
 * @param ip      ip address of server
 * @param port    port of the server
 * @returns       sockaddr_in structure for the given server
 */
struct sockaddr_in make_sv_sockaddr_in(char* ip, int port);

/**
 * Makes sockaddr_in structure of this host.
 *
 * INADDR_ANY is used as IP address.
 *
 * @param port    port of the server
 * @returns       sockaddr_in structure for the given server
 */
struct sockaddr_in make_my_sockaddr_in(int port);

#endif
