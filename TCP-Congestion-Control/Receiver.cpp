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

  int send_socket = socket(AF_INET, SOCK_DGRAM, 0);
  int recv_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (send_socket < 0 || recv_socket < 0) {
    perror("[Fatal] Fail to create socket");
    exit(1);
  }

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(RECEIVER_PORT),
  };
  inet_aton(RECEIVER_ADDR, &addr.sin_addr);
  if (bind(recv_socket, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
    perror("[Fatal] Fail to bind addr");
    exit(1);
  }
  addr.sin_port = htons(AGENT_PORT);
  inet_aton(AGENT_ADDR, &addr.sin_addr);
  if (connect(send_socket, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
    perror("[Fatal] Fail to connect to agent");
    exit(1);
  }

  int fd = open(argv[1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
  if (fd < 0) {
    perror("[Fatal] Fail to create output file");
    exit(1);
  }

  TCPHeader ack_packet;
  ack_packet.dest_port = htons(SENDER_PORT);
  ack_packet.type = ACK;
  inet_aton(SENDER_ADDR, &ack_packet.dest_addr);

  const unsigned int rwnd = 32;
  unsigned int recv_base = 0;
  char recv_buf[rwnd][2000] = {0};
  bool recvd[rwnd] = {false};
  int recvn = 0;
  char buf[2000];
  while (int recv_size = recv(recv_socket, buf, 2000, 0)) {
    if (recv_size < 0) {
      perror("Error while receiving data");
      continue;
    }

    TCPHeader *tcp_header = (TCPHeader*)buf;
    unsigned int seq = tcp_header->seq;

    switch (tcp_header->type) {
    case DATA:
      if (seq > recv_base + rwnd) {
        // Buffer overflow
        printf("drop\tdata\t#%u\n", seq);
        if (recvn == rwnd) {
          for (int i = 0; i < rwnd; ++i) {
            recvd[i] = false;
            write(fd, recv_buf[i] + sizeof(TCPHeader),
                  ((TCPHeader*)recv_buf[i])->payload_size);
          }
          recv_base += rwnd;
          recvn = 0;
          printf("flush\n");
        }
      } else {
        int id = seq - recv_base - 1;
        if (id < 0 || recvd[id]) {
          printf("ignr\tdata\t#%u\n", seq);
        } else {
          recvd[id] = true;
          ++recvn;
          memcpy(recv_buf[id], buf, recv_size);
          printf("recv\tdata\t#%u\n", seq);
        }
        ack_packet.ack = seq;
        send(send_socket, &ack_packet, sizeof(ack_packet), 0);
        printf("send\tack\t#%u\n", seq);
      }
      break;
    case FIN:
      printf("recv\tfin\n");
      ack_packet.type = FINACK;
      send(send_socket, &ack_packet, sizeof(ack_packet), 0);
      printf("send\tfinack\n");
      for (int i = 0; i < rwnd && recvd[i]; ++i) {
        write(fd, recv_buf[i] + sizeof(TCPHeader),
              ((TCPHeader*)recv_buf[i])->payload_size);
      }
      printf("flush\n");
      goto Exit;
    default:
      // Ignore
      break;
    }
  }
Exit:
  close(fd);
  close(send_socket);
  close(recv_socket);
  return 0;
}
