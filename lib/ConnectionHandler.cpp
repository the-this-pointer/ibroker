#include <asio/ip/tcp.hpp>
#include "ConnectionHandler.h"
#include "MessagePacket.h"
#include "Logger.h"

using namespace thisptr::broker;

void ClientConnection::initialize() {
  setHandler(this->shared_from_this());
  m_status = WaitMessage;
  recv(MessageIndicatorLength);
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

bool ClientConnection::onDataReceived(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
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
  std::shared_ptr<MessagePacket> packet = std::make_shared<MessagePacket>(msg, true);
  packet->fromString(m_data);
  m_data.clear();

  LT("[socket] message received, id: {}, type: {}, size: {}", *&msg.header.id, *&msg.header.type, *&msg.header.size);

  std::string p{(const char*)msg.body.data(), msg.header.size};
  switch (msg.header.type) {
    case queueDeclare:
    {
      size_t pos;
      if ((pos = p.find(',')) == std::string::npos) {
        LE("[socket] invalid payload for declare queue, id: {}", *&msg.header.id);
        send(MessagePacket::getResponsePacket(msg, MessageResult_t::rej));
        break;
      }
      const std::string name = p.substr(0, pos);
      const std::string key = p.substr(pos+1);
      LT("[socket] declare queue: {}, key: {}", name, key);
      QueueManager::instance()->newQueue(name, key);
      break;
    }
    case queueBind:
    {
      std::shared_ptr<Queue> q = QueueManager::instance()->bind(p);
      if (!q) {
        LE("[socket] queue not bound yet, id: {}, queue name: {}", *&msg.header.id, p);
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

      size_t pos;
      if ((pos = p.find(',')) == std::string::npos) {
        LE("[socket] invalid message payload received!, id: {}", *&msg.header.id);
        send(MessagePacket::getResponsePacket(msg, MessageResult_t::rej));
        break;
      }
      const std::string key = p.substr(0, pos);
      const std::string payload = p.substr(pos + 1);
      if (key.empty() || payload.empty())
      {
        LE("[socket] invalid message payload received #2!, id: {}", *&msg.header.id);
        send(MessagePacket::getResponsePacket(msg, MessageResult_t::rej));
        break;
      }
      QueueManager::instance()->publish(key, packet);
      break;
    }
  }

  m_status = WaitMessage;
  recv(MessageIndicatorLength);

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
