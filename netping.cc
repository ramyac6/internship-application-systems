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
  std::cerr << "usage: " << progname << " hostname port" << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  // check right number of args
  if (argc != 4) {
    Usage(argv[0]);
  }

  // pull port from command line args
  unsigned short port = 0;
  if (sscanf(argv[2], "%hu", &port) != 1) {
    Usage(argv[0]);
  }

  // Get an appropriate sockaddr structure.
  struct sockaddr_storage addr;
  size_t addrlen;
  if (!LookupName(argv[1], port, &addr, &addrlen)) {
    Usage(argv[0]);
  }

  // Connect to remote host
  int socket_fd;
  if (!Connect(addr, addrlen, &socket_fd)) {
    Usage(argv[0]);
  }

  // Read the local file
  int fd_file = open(argv[3], O_RDONLY);
  if (fd_file < 0) {
    std::cerr << "ERROR: open" << argv[3] << " failed." << std::endl;
    Usage(argv[0]);
  }

  // Read from remote host
  char readbuf[BUFFER_SIZE];
  int result;
  while (true) {
    result = read(fd_file, readbuf, BUFFER_SIZE - 1);
    if (result == 0) {
      std::cerr << "ERROR: socket closed early" << std::endl;
      close(fd_file);
      close(socket_fd);
      return EXIT_FAILURE;
    }
    if (result == -1) {
      if (errno == EINTR || errno == EAGAIN) {
        continue;
      } else {
        std::cerr << "ERROR: socket reading failure: "
                << strerror(errno) << std::endl;
        close(fd_file);
        close(socket_fd);
        return EXIT_FAILURE;
      }
    }
    readbuf[result] = '\0';
    std::cout << readbuf;
    break;
  }

  while (true) {
    int write_result = write(socket_fd, readbuf, result);
    if (write_result == 0) {
      std::cerr << "ERROR: socket closed early" << std::endl;
      close(fd_file);
      close(socket_fd);
      return EXIT_FAILURE;
    }
    if (write_result == -1) {
      if (errno == EINTR || errno == EAGAIN) {
        continue;
      } else {
        std::cerr << "ERROR: socket writing failure: " <<
              strerror(errno) << std::endl;
        close(fd_file);
        close(socket_fd);
        return EXIT_FAILURE;
      }
    }
    break;
  }

  // Clean up.
  close(fd_file);
  close(socket_fd);
  return EXIT_SUCCESS;
}


