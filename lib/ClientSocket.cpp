#include "ClientSocket.h"

using namespace thisptr;
using namespace thisptr::broker;
using namespace thisptr::net;
using namespace std::chrono_literals;

ClientSocket::ClientSocket() : ::thisptr::net::AsyncTcpClient<ClientSocket>()
{}

void ClientSocket::initialize() {
  setHandler(this->shared_from_this());
  m_status = WaitMessage;
}

void ClientSocket::onConnected(asio::ip::tcp::socket &sock, const std::string &endpoint) {
  std::cout << "[client] connected to " << endpoint.c_str() << std::endl;
  recv(MessageIndicatorLength);
}

void ClientSocket::onDisconnected(asio::ip::tcp::socket &sock) {
  std::cout << "[client] disconnected" << std::endl;
}

bool ClientSocket::onDataReceived(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    if (ec == asio::error::eof || ec == asio::error::connection_reset) {
      close();
      return false;
    }
    std::cerr << "[client] unable to read from socket, ec: " << ec << std::endl;
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

  std::cout << "[client] message received: " << msg << std::endl;
  std::string p{(const char*)msg.body.data(), msg.header.size};

  if (msg.header.type == MessageType::queueResult && msg.body.data_t()[0] == MessageResult_t::rej) {
    std::cout << "[client] message with id: " << msg.header.id << " rejected!" << std::endl;
    return true;
  }
  else if (msg.header.type < MessageType::queueUserType) {
    std::cout << "[client] invalid message type received!" << std::endl;
    return true;
  }

  std::cout << "[client] >> message" << std::endl;
  size_t pos;
  if ((pos = p.find(',')) == std::string::npos) {
    std::cout << "[client] invalid message payload received!" << std::endl;
    return true;
  }
  const std::string key = p.substr(0, pos);
  const std::string msgPayload = p.substr(pos + 1);
  if (key.empty() || msgPayload.empty()) {
    std::cout << "[client] invalid message payload received! #2" << std::endl;
    return true;
  }
  std::cout << "[client] message received: " << msgPayload << std::endl;
  return true;
}

void ClientSocket::onDataSent(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    std::cerr << "[client] unable to write to socket" << std::endl;
  } else
    std::cout << "[client] data sent, len: " << payload.length() << std::endl;
}
