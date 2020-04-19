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
#include <sys/time.h>
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

  // Lookup by calling getaddrinfo().
  std::cout << "Performing DNS Lookup..." << std::endl;
  if ((retval = getaddrinfo(name, nullptr, &hints, &results)) != 0) {
    std::cerr << "getaddrinfo failed: ";
    std::cerr << gai_strerror(retval) << std::endl;
    return false;
  }

  // Set ports to 0 for ping
    if (results->ai_family == AF_INET) {
    struct sockaddr_in *v4addr = (struct sockaddr_in *) results->ai_addr;
    v4addr->sin_port = htons(0);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6 *v6addr = (struct sockaddr_in6 *) results->ai_addr;
    v6addr->sin6_port = htons(0);
  } else {
    std::cerr << "getaddrinfo failed to provide an IPv4 or IPv6 address";
    std::cerr << std::endl;
    return false;
  }

  // Return the first result.
  assert(results != nullptr);
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);

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

void Ping(int sock_fd, char* host, struct sockaddr_storage addr) {
  std::cout << "ping " << host << std::endl;
  struct icmp *packet;
  char sendpacket[64];
  char receivepacket[64];
  int count = 0;
  int error;
  socklen_t size;
  while (true) {
    sleep(1);
    // zeros out the packet
    packet = (struct icmp *)sendpacket;
    packet->icmp_type = ICMP_ECHO;
    packet->icmp_code = 0;
    packet->icmp_seq = count;
    packet->icmp_id = getpid();
    // keeps track of when it was sent
    gettimeofday((struct timeval *)packet->icmp_data, NULL);
    error = sendto(sock_fd, sendpacket, sizeof(sendpacket), 0, reinterpret_cast<sockaddr*>(&addr), size);
    
    if (error < 0) {
      std::cerr << "Error in sending packet #" << count << std::endl;
      continue;
    } else {
      count++;
    }
    error = recvfrom(sock_fd,
                              receivepacket,
                              sizeof(receivepacket),
                              0,
                              reinterpret_cast<sockaddr*>(&addr),
                              &size);
        
        if(error < 0) {
            perror("recvfrom error");
            return;
        }
        
        std::cout << "Sent ICMP Echo Requests to " << host << ": Received, time: " << std::endl;

  }

}
