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
    cerr << "Usage: " << argv[0] << " [Data file to send]" << "\n";
    exit(1);
  }

  int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket < 0) {
    perror("[Fatal] Fail to create socket");
    exit(1);
  }

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(SENDER_PORT),
  };
  inet_aton(SENDER_ADDR, &addr.sin_addr);
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

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("[Fatal] Fail to open data file");
    exit(1);
  }

  char packet[2000];
  TCPHeader tcp_header;
  inet_aton(RECEIVER_ADDR, &tcp_header.dest_addr);
  tcp_header.dest_port = htons(RECEIVER_PORT);

  unsigned int seq = 0;
  char buf[1024];
  while (int read_size = read(fd, buf, 1000)) {
    if (read_size < 0) {
      perror("Error while reading data file");
      continue;
    }
    ++seq;

    tcp_header.type = DATA;
    tcp_header.seq = seq;
    unsigned int packet_size = make_packet(&tcp_header, buf, read_size, packet);
    int sent_size = send(udp_socket, packet, packet_size, 0);
    if (sent_size < 0 || sent_size != packet_size) {
      perror("Error while sending data file");
    }
    recv(udp_socket, packet, 2000, 0);
    TCPHeader *ack_packet = (TCPHeader*)packet;
    
  }
  tcp_header.type = FIN;
  int sent_size = send(udp_socket, &tcp_header, sizeof(tcp_header), 0);


  close(fd);
  close(udp_socket);
  return 0;
}
