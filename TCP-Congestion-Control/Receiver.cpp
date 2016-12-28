#include <cstdlib>
#include <cstdio>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <netdb.h>

#include "TCP.h"

using namespace std;

int main(int argc, char *argv[]) {
  unsigned short listen_port = 7002;
  char sender_addr[20] = "127.0.0.1";
  unsigned short sender_port = 7000;
  unsigned int buffer_size = 32;
  char *file_name = NULL;

  static struct option long_options[] = {
    {"listen-port",   required_argument,    NULL,   'l'},
    {"sender",        required_argument,    NULL,   's'},
    {"buffer-size",   required_argument,    NULL,   'b'},
    {"help",          no_argument,          NULL,   'h'},
    {0, 0, 0, 0}
  };

  while (true) {
    int option_index;
    int c = getopt_long_only(argc, argv, "", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
    case 'l':
      listen_port = atoi(optarg);
      break;
    case 's':
      {
        int i = 0;
        while (optarg[i] != '\0' && optarg[i] != ':')
          ++i;
        if (optarg[i] == ':') {
          optarg[i] = '\0';
          strcpy(sender_addr, optarg);
          sender_port = atoi(&optarg[i+1]);
        }
      }
      break;
    case 'b':
      buffer_size = atoi(optarg);
      break;
    case 'h':
    case '?':
      fprintf(stderr, "Usage: %s [-l listen_port] "
              "[-s sender_addr] [-b buffer_size] [File path]\n", argv[0]);
      exit(1);
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Usage: %s [-l listen_port] "
            "[-s sender_addr] [-b buffer_size] [File path]\n", argv[0]);
    exit(1);
  }
  file_name = argv[optind];

  int recv_socket = socket(AF_INET, SOCK_DGRAM, 0);
  int send_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (recv_socket < 0 || send_socket < 0) {
    perror("[Fatal] Fail to create socket");
    exit(1);
  }

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(listen_port),
  };
  inet_aton("127.0.0.1", &addr.sin_addr);
  if (bind(recv_socket, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
    perror("[Fatal] Fail to bind addr");
    exit(1);
  }

  int fd = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0666);
  if (fd < 0) {
    perror("[Fatal] Fail to create output file");
    exit(1);
  }

  TCPHeader ack_packet;
  ack_packet.dest_port = htons(sender_port);
  ack_packet.type = ACK;
  struct hostent *host = gethostbyname(sender_addr);
  ack_packet.dest_addr = *(struct in_addr*)host->h_addr;

  const unsigned int rwnd = max(1u, buffer_size);
  unsigned int recv_base = 0;
  char recv_buf[rwnd][2000] = {0};
  bool recvd[rwnd] = {false};
  int recvn = 0;
  char buf[2000];
  struct sockaddr src_addr;
  socklen_t addrlen = sizeof(struct sockaddr);
  while (int recv_size = recvfrom(recv_socket, buf, 2000, 0, &src_addr, &addrlen)) {
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
        sendto(send_socket, &ack_packet, sizeof(ack_packet), 0, &src_addr, addrlen);
        printf("send\tack\t#%u\n", seq);
      }
      break;
    case FIN:
      printf("recv\tfin\n");
      ack_packet.type = FINACK;
      sendto(send_socket, &ack_packet, sizeof(ack_packet), 0, &src_addr, addrlen);
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
