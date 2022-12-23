#ifndef IBROKER_MESSAGE_H
#define IBROKER_MESSAGE_H

#include <string>
#include <vector>
#include <cstdint>

#define MESSAGE_SIZE                256
#define MESSAGE_SIZE_TYPE           uint16_t
#define MESSAGE_INDICATOR           0x1B
#define MESSAGE_INDICATOR_2         0x1B

#define MAKE_WORD(a,b) ((a & 0xff) | (b & 0xff) << 8)
#define LOWER(w) (w & 0xff)
#define HIGHER(w) ((w >> 8) & 0xff)

enum MessageType: uint8_t {
  ping = 0x00,
  queueDeclare = 0x01,
  queueBind = 02,
  message = 03,
  ack = 04,
  rej = 05,
};

const uint8_t indicator[2] {MESSAGE_INDICATOR, MESSAGE_INDICATOR_2};

struct Message {
  MESSAGE_SIZE_TYPE       size {0x00};
  MessageType             type {MessageType::ping};
  uint8_t                 payload[MESSAGE_SIZE] {0};
} __attribute__((packed));

class MessagePacket {
public:
  explicit MessagePacket(Message& msg, bool includeIndicators = false) : m_msg(msg), m_includeMsgIndicators(includeIndicators) {}

  void fromString(std::string& buf)
  {
    uint8_t offset = 0;
    auto bpSize = buf.size() - sizeof m_msg.size - sizeof m_msg.type - 1;
    if (buf[0] == MESSAGE_INDICATOR) {
      offset += 2;
      bpSize -= 2;
    }
    auto end{std::min(sizeof m_msg.payload, bpSize)};
    m_msg.size = sizeof(m_msg.type) + end;
    m_msg.type = (MessageType)buf[offset + offsetof(Message, type)];
    std::copy(buf.begin() + offset + offsetof(Message, payload), buf.begin() + offset + offsetof(Message, payload) + end, m_msg.payload);
    if (bpSize < sizeof m_msg.payload) {
      std::fill(&m_msg.payload[bpSize], m_msg.payload + sizeof m_msg.payload, 0);
    }
  }

  explicit operator std::string() const {
    std::string str;
    if (!m_includeMsgIndicators)
      str.resize(sizeof m_msg.size + sizeof m_msg.type + std::min((int)m_msg.size, MESSAGE_SIZE));
    else
      str.resize(2 + sizeof m_msg.size + sizeof m_msg.type + std::min((int)m_msg.size, MESSAGE_SIZE));

    int i = 0;
    if (m_includeMsgIndicators)
    {
      str[i++] = MESSAGE_INDICATOR;
      str[i++] = MESSAGE_INDICATOR_2;
    }
    str[i++] = LOWER(m_msg.size);
    str[i++] = HIGHER(m_msg.size);
    str[i++] = m_msg.type;
    for(int j = 0; j < m_msg.size; j++)
      str[i++] = m_msg.payload[j];

    return str;
  }

  void setIncludeMessageIndicators(bool include) { m_includeMsgIndicators = include; }
  constexpr std::size_t size() const { return m_msg.size; }
private:
  bool m_includeMsgIndicators {false};
  Message& m_msg;
};

#endif //IBROKER_MESSAGE_H
