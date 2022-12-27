#include <asio/ip/tcp.hpp>
#include "ConnectionHandler.h"
#include "Message.h"

using namespace thisptr::broker;

void ClientSocket::initialize() {
  setHandler(this->shared_from_this());
  m_status = WaitMessage;
  recv(MessageIndicatorLength);
}

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

  switch (m_status) {
    case WaitMessage: {
      if (MessageIndicatorLength > payload.length())
      {
        recv(MessageIndicatorLength);
        return true;
      }

      uint8_t offset = 0;
      for (; offset < MessageIndicatorLength;)
      {
        if (payload[offset] == MessageIndicator[offset])
          offset++;
        else
        {
          offset = 0;
          break;
        }
      }
      if (offset == 0)
      {
        recv(MessageIndicatorLength);
        return true;
      }
      m_status = ReadHeader;
      recv(sizeof(MessageHeader_t));
      return true;
    }
    case ReadHeader: {
      m_data.clear();
      m_data.append(payload);

      auto bodySize = (MessageSize_t)payload[offsetof(Message_t, header) + offsetof(MessageHeader_t, size)];
      if (bodySize > MaxMessageSize)
        bodySize = MaxMessageSize;

      m_status = ReadBody;
      recv(bodySize);
      return true;
    }
    case ReadBody: {
      m_data.append(payload);

      // rest of it handled at bottom lines...
    }
  }

  Message msg;
  std::shared_ptr<MessagePacket> packet = std::make_shared<MessagePacket>(msg);
  packet->fromString(m_data);
  m_data.clear();

  std::cout << "[socket] message received, size: " << msg << std::endl;

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
    case queueMessage: {
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
    default:
      std::cout << "[socket] invalid message type received!" << std::endl;
      break;
  }

  m_status = WaitMessage;
  recv(MessageIndicatorLength);

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
}
