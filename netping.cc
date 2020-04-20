/* Copyright 2020 Ramya Challa [legal/copyright] */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>

#include <iostream>

#include "./pinguino.h"


void Usage(char *progname) {
  std::cerr << "usage: " << progname << " hostname" << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  // check right number of args
  if (argc != 2) {
    Usage(argv[0]);
  }
  
  // Get an appropriate sockaddr structure.
  struct sockaddr_storage addr;
  char* ipAddr = new (char[NI_MAXHOST * sizeof(char)]);
  if (!LookupName(argv[1], &addr, ipAddr)) {
    Usage(argv[0]);
  }

  // Connect to remote host
  int socket_fd;
  if (!CreateRawSocket(addr, &socket_fd)) {
    Usage(argv[0]);
  }

  // handles the interrupt
  signal(SIGINT, Interrupt);

  // ping
  std::cout << "PING " << argv[1] << " (" << ipAddr << ")" <<std::endl;
  Ping(socket_fd, ipAddr);

  // Clean up.
  close(socket_fd);
  delete ipAddr;
  return EXIT_SUCCESS;
}
