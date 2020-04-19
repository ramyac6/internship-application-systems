/* Copyright 2020 Ramya Challa [legal/copyright]
 * Ramya Challa
 * 1938971
 * ramyac6@cs.washington.edu
 * CSE 333 Winter 2020
 */

#ifndef CONNECT_H_
#define CONNECT_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

// gets the appropriate sockaddr struct through output parameter
// returns true if successful, returns false if an error occured
bool LookupName(char *name,
                struct sockaddr_storage *ret_addr);

// connects the socket to the remote host
// returns socket descriptor through output parameter
// returns true if connected, returns false if error occured
bool CreateRawSocket(const struct sockaddr_storage &addr,
             int *ret_fd);


// pings the passed in host
void Ping(int sock_fd, char* host, struct sockaddr_storage addr);

#endif  // CONNECT_H_
