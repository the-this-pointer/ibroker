#include <asio/ip/tcp.hpp>
#include "ConnectionHandler.h"
#include "Message.h"

using namespace thisptr::broker;

void ServerHandler::onDisconnected(asio::ip::tcp::socket &sock) {
  std::cout << "[server] client disconnected" << std::endl;
  m_connections.erase(&sock);
}

bool ServerHandler::onDataReceived(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    std::cerr << "[server] unable to read from socket, ec: " << ec << std::endl;
    return false;
  }

  std::cout << "[server] data received: " << payload.c_str() << std::endl;
  m_data.append(payload);

  size_t pos = m_data.find(MESSAGE_INDICATOR);
  if (pos == std::string::npos || (m_data.length() > pos+1 && m_data[pos+1] != MESSAGE_INDICATOR_2)) {
    m_data.clear();
    return true;
  }
  pos += 2; // skip message indicators

  uint16_t size = MAKE_WORD(m_data[pos+offsetof(Message, size)], m_data[pos+offsetof(Message, size)+1]);
  if (pos + size >= m_data.length())
    return true;

  std::string packetData = m_data.substr(pos, pos + sizeof(MESSAGE_SIZE_TYPE) + sizeof(MessageType) + size);
  if (m_data.length() >= pos + sizeof(MESSAGE_SIZE_TYPE) + sizeof(MessageType) + size + 1)
    m_data = m_data.substr(pos + sizeof(MESSAGE_SIZE_TYPE) + sizeof(MessageType) + size + 1);
  else
    m_data.clear();
  std::cout << "[server] current data frame: " << packetData << std::endl;
  std::cout << "[server] current data buffer: " << m_data << std::endl;

  Message msg;
  std::shared_ptr<MessagePacket> packet = std::make_shared<MessagePacket>(msg);
  packet->fromString(packetData);

  std::cout << "[server] message received, size: " << msg.size << ", type: " << msg.type << ", payload: " << msg.payload << std::endl;
  std::string p{(const char*)msg.payload, msg.size - sizeof msg.type};
  switch (msg.type) {
    case ping:
      break;
    case queueDeclare: {
      std::cout << "[server] declare queue" << std::endl;
      size_t pos;
      if ((pos = p.find(',')) == std::string::npos) {
        // TODO reject the request
        break;
      }
      const std::string name = p.substr(0, pos);
      const std::string key = p.substr(pos+1);
      std::cout << "[server] declare queue: " << name << ", key: " << key << std::endl;
      QueueManager::instance()->newQueue(name, key);
      break;
    }
    case queueBind:
    {
      std::cout << "[server] bind queue" << std::endl;
      std::shared_ptr<Queue> q = QueueManager::instance()->bind(p);
      if (!q) {
        // TODO reject the request
        break;
      }
      m_connections[&sock]->setQueue(q);
      break;
    }
    case message:
    {
      std::cout << "[server] message" << std::endl;
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
  return true;
}

void ServerHandler::onDataSent(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    std::cerr << "[server] unable to write to socket" << std::endl;
  } else
    std::cout << "[server] data sent, len: " << payload.length() << std::endl;
}

void ServerHandler::onNewConnection(asio::ip::tcp::socket &sock) {
  std::cout << "[server] on new connection" << std::endl;
  auto socket = std::make_shared<ClientSocket>(this->shared_from_this(), sock);
  m_connections[&sock] = socket;

  socket->send("hi from server.");
  socket->recv();
}
