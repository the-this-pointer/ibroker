#include <asio/ip/tcp.hpp>
#include "ConnectionHandler.h"
#include "MessagePacket.h"
#include "Logger.h"

using namespace thisptr::broker;

void ClientConnection::initialize() {
  setHandler(this->shared_from_this());
  waitForMessage();
}

void ClientConnection::onDisconnected(asio::ip::tcp::socket &sock) {
  LI("[socket] disconnected");
}

void ClientConnection::onDataSent(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    LE("[socket] unable to write to socket");
  } else
    LD("[socket] data sent, len: {}", payload.length());
}

bool ClientConnection::onDataReceived(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &data) {
  if (ec) {
    if (ec == asio::error::eof || ec == asio::error::connection_reset) {
      close();
      return false;
    }
    LE("[socket] unable to read from socket, ec: {}, {}", ec.value(), ec.message());
    return false;
  }

  switch (m_status) {
    case WaitMessage: {
      if (MessageIndicatorLength > data.length())
      {
        waitForMessage();
        return true;
      }

      uint8_t offset = 0;
      for (; offset < MessageIndicatorLength;)
      {
        if (data[offset] == MessageIndicator[offset])
          offset++;
        else
        {
          offset = 0;
          break;
        }
      }
      if (offset == 0)
      {
        waitForMessage();
        return true;
      }
      readHeader();
      return true;
    }
    case ReadHeader: {
      m_data.clear();
      m_data.append(data);

      auto bodySize = (MessageSize_t)data[offsetof(Message_t, header) + offsetof(MessageHeader_t, size)];
      if (bodySize > MaxMessageSize)
        bodySize = MaxMessageSize;

      readBody(bodySize);
      return true;
    }
    case ReadBody: {
      m_data.append(data);

      // rest of it handled at bottom lines...
    }
  }

  Message msg;
  std::shared_ptr<MessagePacket> packet = std::make_shared<MessagePacket>(msg, true);
  packet->fromString(m_data);
  m_data.clear();

  LT("[socket] message received, id: {}, type: {}, size: {}", *&msg.header.id, *&msg.header.type, *&msg.header.size);

  switch (msg.header.type) {
    case queueDeclare:
    {
      std::string name{(const char*)msg.body.data(), msg.header.size};
      const std::string key = msg.header.topic;
      if (key.empty()) {
        LE("[socket] empty topic received, id: {}, queue name: {}", *&msg.header.id, name);
        send(MessagePacket::getResponsePacket(msg, MessageResult_t::rej));
        break;
      }
      LT("[socket] declare queue: {}, key: {}", name, key);
      QueueManager::instance().newQueue(name, key);
      break;
    }
    case queueBind:
    {
      std::string name{(const char*)msg.body.data(), msg.header.size};
      std::shared_ptr<Queue> q = QueueManager::instance().bind(name);
      if (!q) {
        LE("[socket] queue not bound yet, id: {}, queue name: {}", *&msg.header.id, name);
        send(MessagePacket::getResponsePacket(msg, MessageResult_t::rej));
        break;
      }
      setQueue(q);
      break;
    }
    default:
    {
      if (msg.header.type < MessageType::queueUserType)
      {
        LE("[socket] reserved message type for future use!, id: {}, type: {}", *&msg.header.id, *&msg.header.type);
        send(MessagePacket::getResponsePacket(msg, MessageResult_t::rej));
        break;
      }

      std::string body{(const char*)msg.body.data(), msg.header.size};
      if (msg.header.topic.empty() || body.empty())
      {
        LE("[socket] invalid message data received #2!, id: {}", *&msg.header.id);
        send(MessagePacket::getResponsePacket(msg, MessageResult_t::rej));
        break;
      }
      QueueManager::instance().publish(msg.header.topic, packet);
      break;
    }
  }

  waitForMessage();

  return true;
}

void ServerHandler::onDisconnected(asio::ip::tcp::socket &sock) {
  LI("[server] disconnected");
  m_connections.erase(&sock);
}

bool ServerHandler::onDataReceived(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    LE("[server] unable to read from socket, ec: {}, {}", ec.value(), ec.message());
    return false;
  } else if (m_connections.find(&sock) == m_connections.end()) {
    LE("[server] connection does not exists.");
    return false;
  }

  LT("[server] data received, len: {}", payload.length());
  return true;
}

void ServerHandler::onDataSent(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    LE("[server] unable to write to socket, ec: {}, {}", ec.value(), ec.message());
  } else
    LT("[server] data sent, len: {}", payload.length());
}

void ServerHandler::onNewConnection(asio::ip::tcp::socket &sock) {
  LT("[server] new connection accepted");
  auto socket = std::make_shared<ClientConnection>(sock);
  socket->initialize();
  m_connections[&sock] = socket;
}
