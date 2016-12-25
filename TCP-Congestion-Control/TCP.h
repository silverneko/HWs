#pragma once
#ifndef __TCP_H__
#define __TCP_H__

#include <string.h>
#include <sys/socket.h>

enum PacketType {
  DATA,
  ACK,
  FIN,
  FINACK
};

struct TCPHeader {
  struct in_addr dest_addr;
  uint16_t dest_port;
  PacketType type;
  uint32_t seq;
  uint32_t ack;
  uint32_t payload_size;
};

unsigned int make_packet(TCPHeader *header, const char *payload,
                         unsigned int size, char *packet) {
  header->payload_size = size;
  memcpy(packet, header, sizeof(TCPHeader));
  memcpy(packet + sizeof(TCPHeader), payload, size);
  return sizeof(TCPHeader) + size;
}

#endif
