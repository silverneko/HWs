#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "Ports.h"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " [Data file to send]" << "\n";
    exit(1);
  }

  int udp_socket;
  struct sockaddr_in addr;

  udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket < 0) {
    perror("[Fatal] Fail to create socket");
    exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(AGENT_PORT1);
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

  char buf[1024];
  while (int size_read = read(fd, buf, 1000)) {
    if (size_read < 0) {
      perror("Error while reading data file");
      continue;
    }
    int size_sent = send(udp_socket, buf, size_read, 0);
    if (size_sent < 0 || size_sent != size_read) {
      perror("Error while sending data file");
    }
  }

  close(fd);
  close(udp_socket);
  return 0;
}
