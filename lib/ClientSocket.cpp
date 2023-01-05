#include "ClientSocket.h"
#include "Logger.h"

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
  LI("[client] connected to: {}", endpoint);
  recv(MessageIndicatorLength);
}

void ClientSocket::onDisconnected(asio::ip::tcp::socket &sock) {
  LI("[client] disconnected");
}

bool ClientSocket::onDataReceived(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &data) {
  if (ec) {
    if (ec == asio::error::eof || ec == asio::error::connection_reset) {
      close();
      return false;
    }
    LE("[client] unable to read from socket, ec: {}, {}", ec.value(), ec.message());
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
  std::shared_ptr<MessagePacket> packet = std::make_shared<MessagePacket>(msg);
  packet->fromString(m_data);
  m_data.clear();

  LT("[client] message received, id: {}, type: {}, size: {}", *&msg.header.id, *&msg.header.type, *&msg.header.size);
  if (msg.header.type == MessageType::queueResult && msg.body.data_t()[0] == MessageResult_t::rej) {
    LE("[client] message rejected, id: {}", *&msg.header.id);
    return true;
  }
  else if (msg.header.type < MessageType::queueUserType) {
    LE("[client] reserved message type for future use!, id: {}, type: {}", *&msg.header.id, *&msg.header.type);
    return true;
  }

  std::string msgPayload{(const char*)msg.body.data(), msg.header.size};
  if (msg.header.topic.empty() || msgPayload.empty()) {
    LE("[client] invalid message data received #2!, id: {}", *&msg.header.id);
    return true;
  }
  LD("[client] message received [topic: {}]: {}", msg.header.topic, msgPayload);

  waitForMessage();
  return true;
}

void ClientSocket::onDataSent(asio::ip::tcp::socket &sock, std::error_code ec, const std::string &payload) {
  if (ec) {
    LE("[client] unable to write to socket, ec: {}, {}", ec.value(), ec.message());
  } else
    LT("[client] data sent, len: {}", payload.length());
}
