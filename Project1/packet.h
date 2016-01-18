#include "crc.h"

typedef struct timer_packet {
  float time ; //Associated Time
  int sequence_number ; //Sequence Number of Packet
  int type ; // 0 - Insert; 1 - Delete
}timer_packet;


typedef struct data_packet {
  char payload[1000];
  int sequence_number;
  char FYN;
  crc checksum;
}data_packet;
