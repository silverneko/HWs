#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "Ports.h"
#include "TCP.h"

using namespace std;

int main(int argc, char *argv[]) {
  int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket < 0) {
    perror("[Fatal] Fail to create socket");
    exit(1);
  }

  struct sockaddr_in my_addr = {
    .sin_family = AF_INET,
    .sin_port = htons(AGENT_PORT)
  };
  inet_aton(AGENT_ADDR, &my_addr.sin_addr);
  if (bind(udp_socket, (struct sockaddr*) &my_addr, sizeof(my_addr)) < 0) {
    perror("[Fatal] Fail to bind addr");
    exit(1);
  }

  char buf[2000];
  while (int recv_size = recv(udp_socket, buf, 2000, 0)) {
    if (recv_size < 0) {
      perror("Error while receiving data");
      continue;
    }

    TCPHeader *tcp_header = (TCPHeader*)buf;
    struct sockaddr_in dest_addr = {
      .sin_family = AF_INET,
      .sin_port = tcp_header->dest_port,
      .sin_addr = tcp_header->dest_addr
    };
    int sent_size = sendto(udp_socket, buf, recv_size, 0,
                           (struct sockaddr*) &dest_addr, sizeof(dest_addr));
    if (sent_size < 0 || sent_size != recv_size) {
      perror("Error while sending packet");
    }

    unsigned int seq = tcp_header->seq;
    unsigned int ack = tcp_header->ack;
    switch (tcp_header->type) {
      case DATA:
        printf("get\tdata\t#%u\n", seq);
        printf("fwd\tdata\t#%u\n", seq);
        break;
      case ACK:
        printf("get\tack\t#%u\n", ack);
        printf("fwd\tack\t#%u\n", ack);
        break;
      case FIN:
        printf("get\tfin\n");
        printf("fwd\tfin\n");
        break;
      case FINACK:
        printf("get\tfinack\n");
        printf("fwd\tfinack\n");
        break;
    }
  }

  close(udp_socket);
  return 0;
}
