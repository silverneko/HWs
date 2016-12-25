#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "Ports.h"
#include "TCP.h"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " [Output file name]" << "\n";
    exit(1);
  }

  int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket < 0) {
    perror("[Fatal] Fail to create socket");
    exit(1);
  }

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(RECEIVER_PORT),
  };
  inet_aton(RECEIVER_ADDR, &addr.sin_addr);
  if (bind(udp_socket, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
    perror("[Fatal] Fail to bind addr");
    exit(1);
  }
  addr.sin_port = htons(AGENT_PORT);
  inet_aton(AGENT_ADDR, &addr.sin_addr);
  if (connect(udp_socket, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
    perror("[Fatal] Fail to connect to agent");
    exit(1);
  }

  int fd = open(argv[1], O_CREAT | O_TRUNC | O_WRONLY);
  if (fd < 0) {
    perror("[Fatal] Fail to create output file");
    exit(1);
  }

  TCPHeader ack_packet;
  inet_aton(SENDER_ADDR, &ack_packet.dest_addr);
  ack_packet.dest_port = htons(SENDER_PORT);
  ack_packet.type = ACK;

  char buf[2000];
  while (int recv_size = recv(udp_socket, buf, 2000, 0)) {
    if (recv_size < 0) {
      perror("Error while receiving data");
      continue;
    }

    TCPHeader *tcp_header = (TCPHeader*)buf;
    char *payload = buf + sizeof(TCPHeader);
    unsigned int payload_size = tcp_header->payload_size;

    switch (tcp_header->type) {
    case DATA:
      payload[payload_size] = '\0';
      write(fd, payload, payload_size);
      send(udp_socket, &ack_packet, sizeof(ack_packet), 0);
      break;
    case ACK:
      break;
    case FIN:
      goto Exit;
      break;
    case FINACK:
      break;
    }
  }
Exit:
  close(fd);
  close(udp_socket);
  return 0;
}
