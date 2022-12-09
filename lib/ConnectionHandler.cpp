#include "ConnectionHandler.h"
#include "Message.h"

void thisptr::broker::BrokerConnectionHandler::onConnect() {
  std::cout << "New connection received!" << std::endl;
}

void thisptr::broker::BrokerConnectionHandler::onDisconnect() {
  std::cout << "New connection closed!" << std::endl;
}

void thisptr::broker::BrokerConnectionHandler::onMessage(std::string data) {
  std::cout << "data received, size: " << data.length() << std::endl;
  m_data.append(data);

  size_t pos = m_data.find(MESSAGE_INDICATOR);
  if (pos == std::string::npos || (m_data.length() > pos+1 && m_data[pos+1] != MESSAGE_INDICATOR_2))
    return;

  auto* size = (uint16_t*)(m_data.data() + pos + offsetof(Message, size));
  if (pos + *size < m_data.length())
      return;

  Message msg;
  msg.size = *size;
  msg.type = *(MessageType*)(m_data.data() + pos + offsetof(Message, type));

  uint16_t payloadSize = msg.size - sizeof msg.size - sizeof msg.type;
  std::copy(data.begin() + pos + offsetof(Message, payload),
            data.begin() + pos + offsetof(Message, payload) + payloadSize,
            std::back_inserter(msg.payload));

  std::cout << "message received, size: " << msg.size << ", type: " << msg.type << ", payload: " << msg.payload;
}
