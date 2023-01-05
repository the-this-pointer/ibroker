#ifndef IBROKER_MESSAGEPACKET_H
#define IBROKER_MESSAGEPACKET_H

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include "Message.h"

namespace thisptr {
  namespace broker {
    class MessagePacket {
    public:
      explicit MessagePacket(Message_t& msg, bool includeIndicators = false) : m_msg(msg), m_includeMsgIndicators(includeIndicators) {}
      explicit MessagePacket(Message_t&& msg, bool includeIndicators = false) : m_msg(msg), m_includeMsgIndicators(includeIndicators) {}

      static std::string getResponsePacket(const Message_t& msg, const MessageResult_t result)
      {
        Message_t respMessage;
        respMessage.header.id = msg.header.id;
        respMessage.header.type = queueResult;
        MessageResponse_t ms { result };
        respMessage.setBody(ms );
        return static_cast<std::string>(MessagePacket(respMessage, true));
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

        memcpy((void *)&m_msg.header.size,
               &buf[offset],
               sizeof(MessageSize_t));
        offset += sizeof(MessageSize_t);

        memcpy((void *)&m_msg.header.type,
               &buf[offset],
               sizeof(MessageType_t));
        offset += sizeof(MessageType_t);

        memcpy((void *)&m_msg.header.id,
               &buf[offset],
               sizeof(MessageId_t));
        offset += sizeof(MessageId_t);

        size_t topicStart = offset;
        size_t topicEnd = buf.find(BodyIndicator, topicStart);

        /*
         * We may don't have any topic at packet. So maybe start and end will equal,
         * but we always should have body indicator in our packet.
         */
        if (topicEnd == std::string::npos || topicStart > topicEnd)
          throw InvalidMessageException();

        m_msg.header.topic.clear();
        m_msg.header.topic.resize(topicEnd - topicStart);
        memcpy((void *)m_msg.header.topic.data(), &buf[topicStart], m_msg.header.topic.capacity());

        m_msg.body.allocate(buf.substr(topicEnd + BodyIndicatorLength));
      }

      explicit operator std::string() const {
        std::string str;
        const size_t bodySize = std::min(m_msg.header.size, MaxMessageSize);
        size_t packetSize = m_msg.header.headerSize() + bodySize;
        if (m_includeMsgIndicators)
          packetSize += 2;
        str.resize(packetSize);

        size_t i = 0;
        if (m_includeMsgIndicators)
        {
          for (int k = 0; k < MessageIndicatorLength; k++)
            str[i++] = MessageIndicator[k];
        }

        memcpy((void *) (str.data() + i), &m_msg.header.size, sizeof(MessageSize_t));
        i += sizeof(MessageSize_t);

        memcpy((void *) (str.data() + i), &m_msg.header.type, sizeof(MessageType_t));
        i += sizeof(MessageType_t);

        memcpy((void *) (str.data() + i), &m_msg.header.id, sizeof(MessageId_t));
        i += sizeof(MessageId_t);

        memcpy((void *) (str.data() + i), m_msg.header.topic.data(), m_msg.header.topic.length());
        i += m_msg.header.topic.length();

        memcpy((void *) (str.data() + i), BodyIndicator.data(), BodyIndicatorLength);
        i += BodyIndicatorLength;

        memcpy((void *) (str.data() + i), m_msg.body.data_t(), bodySize);
        i += bodySize;

        return str;
      }

      void setIncludeMessageIndicators(bool include) { m_includeMsgIndicators = include; }
      constexpr std::size_t size() const { return m_msg.header.size; }
    private:
      bool m_includeMsgIndicators {false};
      Message_t& m_msg;
    };
  }
}

#endif //IBROKER_MESSAGEPACKET_H
