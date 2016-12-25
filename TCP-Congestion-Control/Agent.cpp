#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "Ports.h"
#include "TCP.h"

using namespace std;

int main(int argc, char *argv[]) {
  int udp_socket;
  struct sockaddr_in my_addr;

  udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket < 0) {
    perror("[Fatal] Fail to create socket");
    exit(1);
  }

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(AGENT_PORT);
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
    char *payload = buf + sizeof(TCPHeader);
    unsigned int payload_size = tcp_header->payload_size;
    payload[payload_size] = '\0';
    cout << payload << flush;
  }

  close(udp_socket);
  return 0;
}
