/* Copyright 2020 Ramya Challa [legal/copyright] */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#include <iostream>

#include "./pinguino.h"


// This function adapted from an assignment for CSE 333 Systems
// Programming at University of Washington
bool LookupName(char *name,
                struct sockaddr_storage *ret_addr) {
  struct addrinfo hints, *results;
  int retval;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_RAW;

  // Do the lookup by invoking getaddrinfo().
  std::cout << "Performing DNS Lookup..." << std::endl;
  if ((retval = getaddrinfo(name, nullptr, &hints, &results)) != 0) {
    std::cerr << "getaddrinfo failed: ";
    std::cerr << gai_strerror(retval) << std::endl;
    return false;
  }

  // Return the first result.
  assert(results != nullptr);
  memcpy(ret_addr, results, sizeof(*results));

  // Clean up.
  freeaddrinfo(results);
  return true;
}

// This function adapted from an assignment for CSE 333 Systems
// Programming at University of Washington
bool CreateRawSocket(const struct sockaddr_storage &addr,
             int *ret_fd) {
  // Create the socket.
  std::cout << "Creating socket..." << std::endl;
  int socket_fd = socket(addr.ss_family, SOCK_RAW, IPPROTO_ICMP);
  if (socket_fd == -1) {
    std::cerr << "socket() failed: " << strerror(errno) << std::endl;
    return false;
  }
  *ret_fd = socket_fd;
  return true;
}

void Ping(int sock_fd, char* host) {
  std::cout << "ping " << host << std::endl;
}
