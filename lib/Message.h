#ifndef IBROKER_MESSAGE_H
#define IBROKER_MESSAGE_H

#include <vector>
#include <stdint.h>

#define MESSAGE_INDICATOR           0x1B
#define MESSAGE_INDICATOR_2         0x1B

enum MessageType: uint8_t {
  ping = 0x00,
  queueDeclare = 0x01,
  queueBind = 02,
  ack = 03,
  rej = 04,
  collect = 05,
  message = 06
};

const uint8_t indicator[2] {MESSAGE_INDICATOR, MESSAGE_INDICATOR_2};

struct Message {
  uint16_t                size {0x00};
  MessageType             type {MessageType::ping};
  uint8_t                 payload[256];
} __attribute__((packed));

#endif //IBROKER_MESSAGE_H
