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
#include <fcntl.h>

#include <iostream>

#include "./pinguino.h"

#define BUFFER_SIZE 256

void Usage(char *progname) {
  std::cerr << "usage: " << progname << " hostname" << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  // check right number of args
  if (argc != 2) {
    Usage(argv[0]);
  }


  // TODO some check to see what was passed in, 4,6,url

  // Get an appropriate sockaddr structure.
  struct sockaddr_storage addr;
  if (!LookupName(argv[1], &addr)) {
    Usage(argv[0]);
  }

  // Connect to remote host
  int socket_fd;
  // NOTE: USING RAW SOCKETS REQUIRES SUDO PERMISSIONS
  if (!CreateRawSocket(addr, &socket_fd)) {
    Usage(argv[0]);
  }

  Ping(socket_fd, argv[1], addr);

  // Clean up.
  close(socket_fd);
  return EXIT_SUCCESS;
}


