#include <asio/ip/tcp.hpp>
#include "ConnectionHandler.h"
#include "Message.h"

using namespace thisptr::broker;

void ClientSocket::onDisconnected(asio::ip::tcp::socket &sock) {
  std::cout << "[socket] disconnected" << std::endl;
}

void ClientSocket::onDataSent(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    std::cerr << "[socket] unable to write to socket" << std::endl;
  } else
    std::cout << "[socket] data sent, len: " << payload.length() << std::endl;
}

bool ClientSocket::onDataReceived(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    std::cerr << "[socket] unable to read from socket, ec: " << ec << std::endl;
    return false;
  }

  std::cout << "[socket] data received: " << payload.c_str() << std::endl;
  m_data.append(payload);

  size_t pos = m_data.find(MESSAGE_INDICATOR);
  if (pos == std::string::npos || (m_data.length() > pos+1 && m_data[pos+1] != MESSAGE_INDICATOR_2)) {
    m_data.clear();
    return true;
  }
  pos += 2; // skip message indicators

  auto size = (MessageSize_t)m_data[pos + offsetof(Message_t, header) + offsetof(MessageHeader_t, size)];
  if (pos + size >= m_data.length())
    return true;

  auto packetSize = MessagePacket::getPacketSize(size);
  std::string packetData = m_data.substr(pos, pos + packetSize);
  if (m_data.length() >= pos + packetSize + 1)
    m_data = m_data.substr(pos + packetSize + 1);
  else
    m_data.clear();
  std::cout << "[socket] current data frame: " << packetData << std::endl;
  std::cout << "[socket] current data buffer: " << m_data << std::endl;

  Message msg;
  std::shared_ptr<MessagePacket> packet = std::make_shared<MessagePacket>(msg);
  packet->fromString(packetData);

  std::cout << "[socket] message received, size: " << msg.header.size << ", type: " << msg.header.type << ", payload: " << (const char*)msg.body.data() << std::endl;
  std::string p{(const char*)msg.body.data(), MessagePacket::getSizeFromPacket(msg.header.size)};
  switch (msg.header.type) {
    case queueDeclare: {
      std::cout << "[socket] declare queue" << std::endl;
      size_t pos;
      if ((pos = p.find(',')) == std::string::npos) {
        // TODO reject the request
        break;
      }
      const std::string name = p.substr(0, pos);
      const std::string key = p.substr(pos+1);
      std::cout << "[socket] declare queue: " << name << ", key: " << key << std::endl;
      QueueManager::instance()->newQueue(name, key);
      break;
    }
    case queueBind:
    {
      std::cout << "[socket] bind queue" << std::endl;
      std::shared_ptr<Queue> q = QueueManager::instance()->bind(p);
      if (!q) {
        // TODO reject the request
        break;
      }
      setQueue(q);
      break;
    }
    case message: {
      std::cout << "[socket] message" << std::endl;
      size_t pos;
      if ((pos = p.find(',')) == std::string::npos) {
        // TODO reject the request
        break;
      }
      const std::string key = p.substr(0, pos);
      const std::string payload = p.substr(pos + 1);
      if (key.empty() || payload.empty())
      {
        // TODO reject the request
        break;
      }
      QueueManager::instance()->publish(key, packet);
      break;
    }
  }
  return true;
}

void ServerHandler::onDisconnected(asio::ip::tcp::socket &sock) {
  std::cout << "[server] disconnected" << std::endl;
  m_connections.erase(&sock);
}

bool ServerHandler::onDataReceived(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    std::cerr << "[server] unable to read from socket, ec: " << ec << std::endl;
    return false;
  } else if (m_connections.find(&sock) == m_connections.end())
    return false;

  std::cout << "[server] data received: " << payload.c_str() << std::endl;
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
  auto socket = std::make_shared<ClientSocket>(sock);
  socket->initialize();
  m_connections[&sock] = socket;
  socket->recv();
}
