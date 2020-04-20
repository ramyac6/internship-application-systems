/* Copyright 2020 Ramya Challa [legal/copyright] */

#ifndef CONNECT_H_
#define CONNECT_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

// This function adapted from an assignment for
// CSE 333 Systems Programming at University of Washington
// gets the appropriate sockaddr struct through output parameter
// returns true if successful, returns false if an error occured
bool LookupName(char *name,
                struct sockaddr_storage *ret_addr,
                char* ipAddr);

// This function adapted from an assignment for
// CSE 333 Systems Programming at University of Washington
// connects the socket to the remote host
// returns socket descriptor through output parameter
// returns true if connected, returns false if error occured
bool CreateRawSocket(const struct sockaddr_storage &addr,
                     int *ret_fd);

// pings the passed in host indefinitely until SIGINT is encountered
// at SIGINT, prints out session statistics
void Ping(int sock_fd, char* host);

// handles SIGINT cleanly
void Interrupt(int garbage);

#endif  // CONNECT_H_
