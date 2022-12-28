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

      static MessagePacket getResultPacket(const Message_t& msg, const MessageResult_t result)
      {
        Message_t respMessage;
        respMessage.header.id = msg.header.id;
        respMessage.header.type = queueResult;
        respMessage.header.size = sizeof(MessageResponse_t);
        /*MessageResponse_t ms { MessageResult::ack };*/
        respMessage.setBody(MessageResult::ack );
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
  }
}

#endif //IBROKER_MESSAGEPACKET_H
