#ifndef IBROKER_GLOBALDEFINITIONS_H
#define IBROKER_GLOBALDEFINITIONS_H

#include <cstdint>

constexpr uint8_t MessageIndicatorLength = 2;
constexpr uint8_t MessageIndicator[MessageIndicatorLength] = {0x1B, 0x1B};

typedef enum MessageType: uint8_t {
  queueDeclare  = 0x02,
  queueBind     = 0x03,
  queueMessage  = 0x04,
  queueResult   = 0x05,
} MessageType_t;

typedef enum MessageResult: uint8_t {
  ack = 0x00,
  rej = 0x01,
} MessageResult_t;

typedef uint32_t            MessageId_t;

typedef size_t              MessageSize_t;
constexpr MessageSize_t MaxMessageSize = 256;


#endif //IBROKER_GLOBALDEFINITIONS_H
