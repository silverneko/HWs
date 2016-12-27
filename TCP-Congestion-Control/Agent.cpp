#include <cstdlib>
#include <cstdio>
#include <random>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#include "TCP.h"

using namespace std;

int randint(int lb, int ub) {
  static std::mt19937 RNG(0x5EED);
  return std::uniform_int_distribution<int>(lb, ub)(RNG);
}

float randreal(float lb, float ub) {
  static std::mt19937 RNG(0x5EED);
  return std::uniform_real_distribution<float>(lb, ub)(RNG);
}

bool flip_coin(float true_rate) {
  float x = randreal(0, 1);
  return x < true_rate;
}

int main(int argc, char *argv[]) {
  unsigned short listen_port = 7001;
  float loss_rate = 0.1;

  static struct option long_options[] = {
    {"listen-port",   required_argument,    NULL,   'l'},
    {"loss-rate",     required_argument,    NULL,   'r'},
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
    case 'r':
      loss_rate = atof(optarg);
      break;
    case 'h':
    case '?':
      fprintf(stderr, "Usage: %s [-listen listen_port] [-loss loss_rate] \n", argv[0]);
      exit(1);
    }
  }

  int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket < 0) {
    perror("[Fatal] Fail to create socket");
    exit(1);
  }

  struct sockaddr_in my_addr = {
    .sin_family = AF_INET,
    .sin_port = htons(listen_port)
  };
  inet_aton("127.0.0.1", &my_addr.sin_addr);
  if (bind(udp_socket, (struct sockaddr*) &my_addr, sizeof(my_addr)) < 0) {
    perror("[Fatal] Fail to bind addr");
    exit(1);
  }

  char buf[2000];
  unsigned int recv_count = 0, drop_count = 0;
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

    bool drop_packet = false;
    if (tcp_header->type == DATA) {
      drop_packet = flip_coin(loss_rate);
    }

    if (!drop_packet) {
      int sent_size = sendto(udp_socket, buf, recv_size, 0,
                             (struct sockaddr*) &dest_addr, sizeof(dest_addr));
      if (sent_size < 0 || sent_size != recv_size) {
        perror("Error while sending packet");
      }
    }

    unsigned int seq = tcp_header->seq;
    unsigned int ack = tcp_header->ack;
    switch (tcp_header->type) {
      case DATA:
        ++recv_count;
        printf("get\tdata\t#%u\n", seq);
        if (!drop_packet) {
          printf("fwd\tdata\t#%u,\tloss rate = %f\n", seq,
                 (float)drop_count / recv_count);
        } else {
          ++drop_count;
          printf("drop\tdata\t#%u,\tloss rate = %f\n", seq,
                 (float)drop_count / recv_count);
        }
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
