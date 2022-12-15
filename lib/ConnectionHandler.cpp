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
  if (pos == std::string::npos || (m_data.length() > pos+1 && m_data[pos+1] != MESSAGE_INDICATOR_2)) {
    m_data.clear();
    return;
  }
  pos += 2; // skip message indicators

  uint16_t size = MAKE_WORD(m_data[pos+offsetof(Message, size)], m_data[pos+offsetof(Message, size)+1]);
  if (pos + size >= m_data.length())
      return;

  Message msg;
  std::shared_ptr<MessagePacket> packet = std::make_shared<MessagePacket>(msg);
  packet->fromString(m_data);

  std::cout << "message received, size: " << msg.size << ", type: " << msg.type << ", payload: " << msg.payload << std::endl;
  std::string payload{(const char*)msg.payload, msg.size - sizeof msg.type};
  switch (msg.type) {
    case ping:
      break;
    case queueDeclare: {
      size_t p;
      if ((p = payload.find(',')) == std::string::npos) {
        // TODO reject the request
        break;
      }
      const std::string name = payload.substr(0, p);
      const std::string key = payload.substr(p+1);
      std::cout << "declare queue: " << name << ", key: " << key << std::endl;
      QueueManager::instance()->newQueue(name, key);
      break;
    }
    case queueBind:
    {
      std::cout << "bind queue: " << payload << std::endl;
      std::shared_ptr<Queue> q = QueueManager::instance()->bind(payload);
      if (!q) {
        // TODO reject the request
        break;
      }
      m_queue = q;
      break;
    }
    case message:
    {
      size_t p;
      if ((p = payload.find(',')) == std::string::npos) {
        // TODO reject the request
        break;
      }
      const std::string key = payload.substr(0, p);
      const std::string payload = payload.substr(p+1);
      QueueManager::instance()->publish(key, packet);
      break;
    }
    case ack:
      break;
    case rej:
      break;
  }
}
