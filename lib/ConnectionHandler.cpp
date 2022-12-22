#include <asio/ip/tcp.hpp>
#include "ConnectionHandler.h"
#include "Message.h"

using namespace thisptr::broker;

/*void BrokerConnectionHandler::onMessage(std::string data) {
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
*/

ServerHandler::~ServerHandler() {}

void ServerHandler::onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) {
  if (ec) {
    std::cerr << "[server] unable to read from socket, ec: " << ec << std::endl;
    return;
  }

  std::cout << "[server] data received: " << payload.c_str() << std::endl;
  m_data.append(payload);

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
  std::string p{(const char*)msg.payload, msg.size - sizeof msg.type};
  switch (msg.type) {
    case ping:
      break;
    case queueDeclare: {
      size_t pos;
      if ((pos = p.find(',')) == std::string::npos) {
        // TODO reject the request
        break;
      }
      const std::string name = p.substr(0, pos);
      const std::string key = p.substr(pos+1);
      std::cout << "declare queue: " << name << ", key: " << key << std::endl;
      QueueManager::instance()->newQueue(name, key);
      break;
    }
    case queueBind:
    {
      std::cout << "bind queue: " << p << std::endl;
      std::shared_ptr<Queue> q = QueueManager::instance()->bind(p);
      if (!q) {
        // TODO reject the request
        break;
      }
      m_queue = q;
      break;
    }
    case message:
    {
      size_t pos;
      if ((pos = p.find(',')) == std::string::npos) {
        // TODO reject the request
        break;
      }
      const std::string key = p.substr(0, pos);
      const std::string payload = p.substr(pos+1);
      QueueManager::instance()->publish(key, packet);
      break;
    }
    case ack:
      break;
    case rej:
      break;
  }
}

void ServerHandler::onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) {
  if (ec) {
    std::cerr << "[server] unable to write to socket" << std::endl;
  } else
    std::cout << "[server] data sent, len: " << payload.length() << std::endl;
}

void ServerHandler::onNewConnection(asio::ip::tcp::socket& sock) {
  std::cout << "[server] on new connection" << std::endl;
  auto socket = std::make_shared<AsioTcpSocket<ServerHandler>>(this->shared_from_this(), sock);
  m_connections[&sock] = socket;
}

void ServerHandler::onDisconnected(asio::ip::tcp::socket &sock) {
  std::cout << "[server] on client disconnected" << std::endl;
  m_connections.erase(&sock);
}

