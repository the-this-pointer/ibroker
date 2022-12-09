#ifndef IBROKER_MESSAGE_H
#define IBROKER_MESSAGE_H

#include <vector>
#include <stdint.h>

#define MESSAGE_SIZE                256
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
    str[i++] = LOBYTE(m_msg.size);
    str[i++] = HIBYTE(m_msg.size);
    str[i++] = m_msg.type;
    for(int j = 0; j < m_msg.size; j++)
      str[i++] = m_msg.payload[j];

    return str;
  }

  void setIncludeMessageIndicators(bool include) { m_includeMsgIndicators = include; }
  constexpr std::size_t capacity() const { return sizeof m_msg; }
  constexpr std::size_t size() const { return m_msg.size; }
private:
  bool m_includeMsgIndicators {false};
  Message& m_msg;
};


class MessagePacketPayload {
public:
  explicit MessagePacketPayload(Message& msg) : m_msg(msg) {
    std::fill(m_msg.payload, m_msg.payload + sizeof m_msg.payload, 0);
    m_msg.size = 0;
    m_msg.type = ping;
  }

  explicit MessagePacketPayload(Message& msg, std::string& buf) : m_msg(msg) {
    auto end{std::min(sizeof m_msg.payload, buf.size())};
    std::copy(buf.begin(), buf.begin() + end, m_msg.payload);
    m_msg.size = sizeof(m_msg.type) + buf.length();
    if (buf.size() < sizeof m_msg.payload) {
      std::fill(&m_msg.payload[buf.size()], m_msg.payload + sizeof m_msg.payload, 0);
    }
  }

  MessagePacketPayload(Message& msg, uint8_t* buf, size_t len) : m_msg(msg) {
    auto end{std::min(sizeof m_msg.payload, len)};
    std::copy(buf, buf + end, m_msg.payload);
    m_msg.size = sizeof(m_msg.type) + len;
    if (len < sizeof m_msg.payload) {
      std::fill(&m_msg.payload[len], m_msg.payload + sizeof m_msg.payload, 0);
    }
  }

  explicit operator std::string() const {
    return {reinterpret_cast<const char *>(&m_msg.payload[0]), sizeof m_msg.payload};
  }

  constexpr std::size_t capacity() const { return sizeof m_msg.payload; }
  constexpr std::size_t size() const { return m_msg.size; }
private:
  Message& m_msg;
};

#endif //IBROKER_MESSAGE_H
