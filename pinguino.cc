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
#include <time.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#include <iostream>

#include "./pinguino.h"

#define PACKET_SIZE 64

bool loop = true;  // boolean for infinite loop

void Interrupt(int garbage) {
  loop = false;
}

bool LookupName(char *name,
                struct sockaddr_storage *ret_addr,
                char* ipAddr) {
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

  // This section borrows from angraves "Using getaddrinfo"
  // https://github.com/angrave/SystemProgramming/wiki/Networking%2C-Part-2%3A-Using-getaddrinfo
  struct addrinfo *p;
  char host[256];

  for (p = results; p != NULL; p = p->ai_next) {
    getnameinfo(p->ai_addr, p->ai_addrlen, host,
                sizeof (host), NULL, 0, NI_NUMERICHOST);
    break;
  }

  // pass out ip address
  snprintf(ipAddr, 256, host);

  // Return the first result.
  assert(results != nullptr);
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);

  // Clean up.
  freeaddrinfo(results);
  return true;
}

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
  struct icmp* icmpSend;
  char packetbuf[128];
  struct sockaddr_storage returnAddr;
  struct sockaddr_storage addr;
  socklen_t size = sizeof(returnAddr);

  int ttl_val = 128;
  int count = 0;
  int received = 0;

  int64_t errorCheck;

  // Set timeout duration
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  struct timespec start_ping;
  struct timespec end_ping;
  struct timespec begin;
  struct timespec end;

  // get time we started pinging
  clock_gettime(CLOCK_MONOTONIC, &begin);

  // set socket ttl
  if (setsockopt(sock_fd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) {
    std::cerr << "Setting TTL in socket options failed." << std::endl;
    return;
  }

  // set socket timeout
  setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO,
            (const char*) &timeout, sizeof(timeout));

  std::cout << "Sending ICMP Echo Requests to " << host << std::endl;

  while (loop) {
    // one second delay between pings
    sleep(1);

    // setup icmp packet
    icmpSend = (struct icmp*) packetbuf;
    memset(icmpSend, 0, sizeof(packetbuf));
    icmpSend->icmp_type = ICMP_ECHO;
    icmpSend->icmp_code = 0;
    icmpSend->icmp_seq = count++;
    icmpSend->icmp_id = getpid();

    // get time when packet is sent
    clock_gettime(CLOCK_MONOTONIC, &start_ping);

    // Send ICMP header to server
    errorCheck = sendto(sock_fd,
                        icmpSend,
                        PACKET_SIZE,
                        0,
                        ((struct sockaddr*)(&addr)),
                        size);

    // check if packet was sent
    if (errorCheck < 0) {
      perror("Error using sendto");
    } else {
      // receive packet
      errorCheck = recvfrom(sock_fd,
                            icmpSend,
                            PACKET_SIZE,
                            0,
                            (struct sockaddr*)(&returnAddr),
                            &size);

      // check if packet was received
      if (errorCheck < 0) {
        perror("Error in recvfrom");
        return;
      } else {
        // get time packet was received
        clock_gettime(CLOCK_MONOTONIC, &end_ping);

        // calculate milliseconds from nanoseconds
        double ping_time = ((end_ping.tv_nsec - start_ping.tv_nsec))/1000000.0;
        // calculating milliseconds from seconds
        ping_time += ((end_ping.tv_sec - start_ping.tv_sec) * 1000.0);
        double rtt_msec = ping_time;

        // print time for packet rtt
        std::cout << "64 bytes from " << host << ": icmp_seq=" << count;
        std::cout << " ttl=" << ttl_val << " rtt = " << rtt_msec << " ms.";
        std::cout << std::endl;

        // increment number of packets received
        received++;
      }
    }
  }

  // we encoutered SIGINT
  // get time stopped pinging
  clock_gettime(CLOCK_MONOTONIC, &end);

  // print ping statistics
  std::cout << "----------------ping statistics-----------------" << std::endl;
  double pkt_loss = 1.0*(count-received)/count*100;
  double total_time = ((end.tv_nsec - begin.tv_nsec))/1000000.0;
  total_time += ((end.tv_sec - begin.tv_sec) * 1000.0);
  std::cout << count <<" packets transmitted, " << received << " received, ";
  std::cout << pkt_loss << " percent packet loss, time " << total_time;
  std::cout << " ms" <<   std::endl;
}
