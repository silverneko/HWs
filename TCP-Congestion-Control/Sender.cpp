#include <cstdio>
#include <iostream>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include "Ports.h"
#include "TCP.h"

using namespace std;

#define jiffies raid6_jiffies()

static inline uint32_t raid6_jiffies() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000 + tv.tv_usec/1000;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " [Data file to send]" << "\n";
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
    .sin_port = htons(SENDER_PORT),
  };
  inet_aton(SENDER_ADDR, &addr.sin_addr);
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

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("[Fatal] Fail to open data file");
    exit(1);
  }

  TCPHeader tcp_header;
  tcp_header.dest_port = htons(RECEIVER_PORT);
  tcp_header.type = DATA;
  inet_aton(RECEIVER_ADDR, &tcp_header.dest_addr);

  unsigned int timeout_value = 100;
  unsigned int timer_base = 0;
  bool timer_armed = false;
#define start_timer() {timer_armed = true; timer_base = jiffies;}
#define timeout() (timer_armed && jiffies - timer_base > timeout_value)
#define stop_timer() {timer_armed = false;}

  unsigned int file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  unsigned int max_seq = 1 + (file_size - 1) / 1000;
  unsigned int send_base = 0, next_seq = 1, threshold = 16, cwnd = 1;
  char buf[1024], packet_buf[2000];
  vector<bool> ACKed(max_seq + 1); // seq 0 is not used
  vector<bool> resend(max_seq + 1); // seq 0 is not used

  while (true) {
    if (next_seq > max_seq && send_base == max_seq) {
      break;
    }

    while (next_seq <= max_seq && next_seq - send_base <= cwnd) {
      tcp_header.seq = next_seq;
      int read_size = pread(fd, buf, 1000, (next_seq - 1) * 1000);
      if (read_size < 0) {
        perror("Error while reading data file");
        continue;
      }
      unsigned int packet_size = make_packet(&tcp_header, buf, read_size, packet_buf);
      int sent_size = send(send_socket, packet_buf, packet_size, 0);
      if (resend[next_seq]) {
        printf("resnd\tdata\t#%u,\twinSize = %u\n", next_seq, cwnd);
      } else {
        printf("send\tdata\t#%u,\twinSize = %u\n", next_seq, cwnd);
      }
      if (sent_size < 0 || sent_size != packet_size) {
        perror("Error while sending data file");
      }
      if (!timer_armed) {
        start_timer();
      }
      ACKed[next_seq] = false;
      resend[next_seq] = true;
      ++next_seq;
    }

    int recv_size;
    while ((recv_size = recv(recv_socket, packet_buf, 2000, MSG_DONTWAIT)) > 0) {
      TCPHeader *ack_packet = (TCPHeader*)packet_buf;
      if (ack_packet->type == ACK) {
        unsigned int ack = ack_packet->ack;
        if (ack < next_seq) {
          if (ACKed[ack]) {
            printf("recv\tack\t#%u\t(duplicate)\n", ack);
          } else {
            printf("recv\tack\t#%u\n", ack);
          }
          ACKed[ack] = true;
          while (send_base + 1 < next_seq && ACKed[send_base + 1]) {
            ++send_base;
            cwnd += 1;
            start_timer();
          }
        }
      }
    }

    if (timeout()) {
      stop_timer();
      // Check if there is any unACK packet
      if (send_base + 1 < next_seq) {
        // Retransmit from send_base + 1
        printf("time\tout,\t\tthreshold = %u\n", threshold);
        cwnd = 1;
        next_seq = send_base + 1;
      }
    }
  }

  tcp_header.type = FIN;
  int sent_size = send(send_socket, &tcp_header, sizeof(tcp_header), 0);
  printf("send\tfin\n");
  int recv_size;
  while ((recv_size = recv(recv_socket, packet_buf, 2000, 0)) > 0) {
    TCPHeader *ack_packet = (TCPHeader*)packet_buf;
    if (ack_packet->type == FINACK) {
      printf("recv\tfinack\n");
      break;
    }
  }

  close(fd);
  close(send_socket);
  close(recv_socket);
  return 0;
}
