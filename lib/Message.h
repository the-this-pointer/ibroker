#ifndef IBROKER_MESSAGE_H
#define IBROKER_MESSAGE_H

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include "GlobalDefinitions.h"
#include "TemplateHelpers.h"

typedef struct MessageBody{
  MessageBody() = default;

  explicit MessageBody(MessageSize_t size)
  { allocateBuffer(size); }

  ~MessageBody()
  { reset(); }

  void allocateBuffer(MessageSize_t size)
  {
    reset();
    ptr = new uint8_t[size > MaxMessageSize? MaxMessageSize: size];
  }

  template <typename T,
      thisptr::broker::enable_if_pod<T> = true>
  void allocate(const T& data)
  {
    allocateBuffer(sizeof(T));
    memcpy(ptr, data, sizeof(T));
  }

  template <typename T,
      thisptr::broker::enable_if_string<T> = true>
  void allocate(const T& data)
  {
    allocateBuffer(data.length());
    std::copy(data.begin(),
              data.begin() + data.length(),
              ptr);
  }

  template <typename T,
      thisptr::broker::enable_if_has_serializer<T> = true>
  void allocate(const T& data)
  {
    auto bodySerializer = thisptr::broker::serializer<T>();
    allocate(bodySerializer.serializeSize(data));
    bodySerializer.serialize(data, ptr);
  }

  inline void reset()
  {
    delete[] ptr;
    ptr = nullptr;
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
  MessageType_t           type  { MessageType::queueMessage };
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

  static MessagePacket getResultPacket(const Message_t& msg, const MessageResult_t result)
  {
    Message_t respMessage;
    respMessage.header.id = msg.header.id;
    respMessage.header.type = queueResult;
    respMessage.header.size = sizeof(MessageResult_t);
    return MessagePacket(respMessage, true);
  }

  void fromString(const std::string& buf)
  {
    uint8_t offset = 0;
    for (;m_includeMsgIndicators && offset < MessageIndicatorLength;)
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
    m_msg.body.allocate(buf.substr(offset + sizeof(MessageHeader_t)));
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
