#ifndef IBROKER_MESSAGE_H
#define IBROKER_MESSAGE_H

#include <cstdint>
#include <string>
#include <iostream>
#include "TemplateHelpers.h"

namespace thisptr {
  namespace broker {
    constexpr uint8_t MessageIndicatorLength = 2;
    constexpr uint8_t MessageIndicator[MessageIndicatorLength] = {0x1B, 0x1B};

    typedef enum MessageType: uint16_t {
      queueDeclare  = 0x03,
      queueBind     = 0x04,
      queueResult   = 0x05,
      queueUserType = 0xFF,
    } MessageType_t;

    typedef enum MessageResult: uint8_t {
      ack = 0x01,
      rej = 0x02,
    } MessageResult_t;

    typedef uint32_t            MessageId_t;

    typedef size_t              MessageSize_t;
    constexpr MessageSize_t MaxMessageSize = 256;

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
      void allocate(const T& data, MessageSize_t* size = nullptr)
      {
        MessageSize_t sz = sizeof(T);
        allocateBuffer(sz);
        memcpy(ptr, (void *)data, sizeof(T));
        if (size) *size = sz;
      }

      template <typename T,
          thisptr::broker::enable_if_string<T> = true>
      void allocate(const T& data, MessageSize_t* size = nullptr)
      {
        MessageSize_t sz = data.length();
        allocateBuffer(sz);
        std::copy(data.begin(),
                  data.begin() + data.length(),
                  ptr);
        if (size) *size = sz;
      }

      template <typename T,
          thisptr::broker::enable_if_has_serializer<T> = true>
      void allocate(const T& data, MessageSize_t* size = nullptr)
      {
        size_t sz = serializeSize(data);
        allocateBuffer(sz);
        serializeBody(data, ptr);
        if (size) *size = sz;
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
      MessageType_t           type  { MessageType::queueDeclare };
      MessageId_t             id    { 0x00 };
    } MessageHeader_t;

    typedef struct Message {
      MessageHeader_t      header;
      MessageBody_t        body;

      template <typename T>
      void setBody(const T& data)
      {
        body.allocate(data, &header.size);
      }

      friend std::ostream& operator << (std::ostream& os, const Message& msg)
      {
        os << "Message #" << msg.header.id << ", type: " << msg.header.type << ", size: " << msg.header.size;
        return os;
      }

    } Message_t ;

    typedef struct __attribute__((packed)) {
      MessageResult_t         result { MessageResult::ack };
    } MessageResponse_t;

    template <>
    void serializeBody(const MessageResponse_t& data, void* ptr);
  }
}


#endif //IBROKER_MESSAGE_H
