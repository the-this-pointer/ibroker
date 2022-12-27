#ifndef IBROKER_MESSAGE_H
#define IBROKER_MESSAGE_H

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

constexpr uint8_t MessageIndicatorLength = 2;
constexpr uint8_t MessageIndicator[MessageIndicatorLength] = {0x1B, 0x1B};

typedef enum MessageType: uint8_t {
  queueDeclare = 0x02,
  queueBind = 0x03,
  queueMessage = 0x04,
} MessageType_t;

typedef enum MessageResult: uint8_t {
  ack = 0x00,
  rej = 0x01,
} MessageResult_t;

typedef uint32_t            MessageId_t;

typedef size_t              MessageSize_t;
constexpr MessageSize_t MaxMessageSize = 256;

typedef struct MessageBody{
  MessageBody() = default;

  explicit MessageBody(MessageSize_t size)
  { allocate(size); }

  ~MessageBody()
  { delete[] ptr; }

  void allocate(MessageSize_t size)
  {
    delete[] ptr;
    ptr = new uint8_t[size > MaxMessageSize? MaxMessageSize: size];
  }

  void allocate(const std::string& data, MessageSize_t offset = 0)
  {
    allocate(data.length() - offset);
    std::copy(data.begin() + offset,
              data.begin() + offset + data.length(),
              ptr);
  }

  void* data() const
  { return ptr; }

  uint8_t* data_t()
  { return ptr; }

private:
  uint8_t* ptr {nullptr};
} MessageBody_t;

typedef struct __attribute__((packed)) {
  MessageSize_t           size  { 0x00 };
  MessageType_t           type  { MessageType::message };
  MessageId_t             id    { 0x00 };
} MessageHeader_t;

typedef struct Message {
  MessageHeader_t      header;
  MessageBody_t        body;

  void setBody(const std::string& data)
  {
    header.size = data.length() > MaxMessageSize? MaxMessageSize: data.length();
    body.allocate(data);
  }

  friend std::ostream& operator << (std::ostream& os, const Message& msg)
  {
    os << "Message #" << msg.header.id << ", type: " << msg.header.type << ", size: " << msg.header.size;
    return os;
  }

} Message_t ;

typedef struct __attribute__((packed)) {
  MessageId_t             id    { 0x00 };
  MessageResult_t         result { MessageResult::ack };
} MessageResponse_t;

class MessagePacket {
public:
  explicit MessagePacket(Message_t& msg, bool includeIndicators = false) : m_msg(msg), m_includeMsgIndicators(includeIndicators) {}

  static MessageSize_t getSizeFromPacket(MessageSize_t size)
  {
    return size - sizeof(MessageType_t) - sizeof(MessageId_t) - 1;
  }

  static MessageSize_t getPacketSize(MessageSize_t payloadSize)
  {
    return payloadSize + sizeof(MessageType_t) + sizeof(MessageId_t) - 1;
  }

  void fromString(std::string& buf)
  {
    uint8_t offset = 0;
    for (; offset < MessageIndicatorLength;)
    {
      if (buf[offset] == MessageIndicator[offset])
        offset++;
      else
      {
        offset = 0;
        break;
      }
    }

    memcpy((void *)&m_msg.header, &buf[offset + offsetof(Message_t, header)], sizeof(MessageHeader_t));
    m_msg.body.allocate(buf, offset + sizeof(MessageHeader_t));
  }

  explicit operator std::string() const {
    std::string str;
    if (!m_includeMsgIndicators)
      str.resize(sizeof(MessageHeader_t) + std::min(m_msg.header.size, MaxMessageSize));
    else
      str.resize(2 + sizeof(MessageHeader_t) + std::min(m_msg.header.size, MaxMessageSize));

    int i = 0;
    if (m_includeMsgIndicators)
    {
      for (int k = 0; k < MessageIndicatorLength; k++)
        str[i++] = MessageIndicator[k];
    }

    memcpy((void *) (str.data() + i), &m_msg.header, sizeof(MessageHeader_t));
    i += sizeof(MessageHeader_t);

    for(int j = 0; j < m_msg.header.size; j++)
      str[i++] = (char)m_msg.body.data_t()[j];

    return str;
  }

  void setIncludeMessageIndicators(bool include) { m_includeMsgIndicators = include; }
  constexpr std::size_t size() const { return m_msg.header.size; }
private:
  bool m_includeMsgIndicators {false};
  Message_t& m_msg;
};

#endif //IBROKER_MESSAGE_H
