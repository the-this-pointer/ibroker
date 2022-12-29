
#define CATCH_CONFIG_MAIN
#include <memory>
#include "catch.hpp"
#include "../lib/ConnectionHandler.h"
#include "../lib/QueueManager.h"
#include "../lib/MessagePacket.h"

using namespace thisptr;
using namespace thisptr::broker;
using namespace thisptr::net;
using namespace std::chrono_literals;

auto startServer() {
  std::shared_ptr<ServerHandler> handler = std::make_shared<ServerHandler>();
  auto s = std::make_shared<AsyncTcpServer<ServerHandler>>(handler);

  s->start("127.0.0.1", "7232");
  return s;
}

void stopServer(const std::shared_ptr<AsyncTcpServer<ServerHandler>>& s) {
  // stop server after 3 seconds
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(3000ms);
  s->stop();
}

class ClientSocket: public std::enable_shared_from_this<ClientSocket>,
                    public AsyncConnectionHandlerBase<AsioTcpSocket<ClientSocket>>,
                    public ::thisptr::net::AsyncTcpClient<ClientSocket> {
public:
  typedef enum: uint8_t {
    WaitMessage,
    ReadHeader,
    ReadBody
  } HandlerStatus_t;

  ClientSocket(): ::thisptr::net::AsyncTcpClient<ClientSocket>()
  {}

  void initialize() {
    setHandler(this->shared_from_this());
    m_status = WaitMessage;
  }
  
  void onConnected(asio::ip::tcp::socket& sock, const std::string &endpoint) override {
    std::cout << "[client] connected to " << endpoint.c_str() << std::endl;
    recv(MessageIndicatorLength);
  }

  void onDisconnected(asio::ip::tcp::socket& sock) override {
    std::cout << "[client] disconnected" << std::endl;
  }

  bool onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override {
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

  void onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override {
    if (ec) {
      std::cerr << "[client] unable to write to socket" << std::endl;
    } else
      std::cout << "[client] data sent, len: " << payload.length() << std::endl;
  }
private:
  std::string m_data;
  HandlerStatus_t m_status {WaitMessage};
};

TEST_CASE("message packet test", "[packet]") {
  SECTION("sucess serialization") {
    Message_t msg;
    msg.header.id = 0x01;
    msg.header.type = static_cast<MessageType_t>(MessageType::queueUserType + 1);
    msg.setBody("this is a test!");

    MessagePacket packet(msg, true);
    std::string dataToSend = static_cast<std::string>(packet);

    Message_t msg2;
    MessagePacket packet2(msg2, true);
    packet2.fromString(dataToSend);

    REQUIRE(msg.header.id == msg2.header.id);
    REQUIRE(msg.header.type == msg2.header.type);
    REQUIRE(msg.header.size == msg2.header.size);
    REQUIRE(memcmp(msg.body.data(), msg2.body.data(), msg.header.size) == 0);
  }
  SECTION("failed serialization") {
    Message_t msg;
    msg.header.id = 0x01;
    msg.header.type = static_cast<MessageType_t>(MessageType::queueUserType + 1);
    msg.setBody("this is a test!");

    MessagePacket packet(msg, true);
    std::string dataToSend = static_cast<std::string>(packet);

    Message_t msg2;
    MessagePacket packet2(msg2);
    packet2.fromString(dataToSend);

    REQUIRE_FALSE(msg.header.id == msg2.header.id);
  }
}

TEST_CASE("queue manager test", "[handler]") {
  SECTION("successfull declaration") {
    TestQueueManager qm(QueueManager::instance());
    REQUIRE(qm.queues().size() == 0);
    REQUIRE(qm.queueBindings().size() == 0);

    auto server = startServer();
    std::this_thread::sleep_for(100ms);

    auto c = std::make_shared<ClientSocket>();
    c->initialize();
    c->connect("127.0.0.1", "7232");

    uint8_t id;
    Message msg;
    msg.header.id = id++;
    msg.header.type = queueDeclare;
    std::string data = "camera,takepic";
    msg.setBody(data);

    MessagePacket packet(msg, true);
    c->send(static_cast<std::string>(packet));
    std::this_thread::sleep_for(100ms);

    REQUIRE(qm.queues().size() == 1);
    REQUIRE(qm.queueBindings().size() == 1);
  }
  SECTION("successfull message match") {
    TestQueueManager qm(QueueManager::instance());

    auto server = startServer();
    std::this_thread::sleep_for(1000ms);

    auto c1 = std::make_shared<ClientSocket>();
    auto c2 = std::make_shared<ClientSocket>();
    c1->initialize();
    c2->initialize();
    c1->connect("127.0.0.1", "7232");
    c2->connect("127.0.0.1", "7232");

    // Declare Queue
    uint8_t id;
    Message msgDeclare;
    msgDeclare.header.id = id++;
    msgDeclare.header.type = queueDeclare;
    std::string data = "camera,takepic";
    msgDeclare.setBody(data);

    MessagePacket packetDeclare(msgDeclare, true);
    c1->send(static_cast<std::string>(packetDeclare));
    std::this_thread::sleep_for(100ms);

    // Bind Queue
    Message msgBind;
    msgBind.header.id = id++;
    msgBind.header.type = queueBind;
    std::string queueName = "camera";
    msgBind.setBody(queueName);

    MessagePacket packetBind(msgBind, true);
    c1->send(static_cast<std::string>(packetBind));
    c2->send(static_cast<std::string>(packetBind));
    std::this_thread::sleep_for(100ms);

    // Post Message To Queue
    Message msgMessage;
    msgMessage.header.id = id++;
    msgMessage.header.type = static_cast<MessageType_t>(MessageType::queueUserType + 1);
    std::string messagePayload = "takepic,hi this is sample payload for take picture!";
    msgMessage.setBody(messagePayload);

    MessagePacket packetMessage(msgMessage, true);
    c1->send(static_cast<std::string>(packetMessage));
    std::this_thread::sleep_for(100ms);

    std::cout << "closing c2" << std::endl;
    c2->close();
    std::this_thread::sleep_for(1000ms);

    std::cout << "sending other message by c1..." << std::endl;
    c1->send(static_cast<std::string>(packetMessage));
    std::this_thread::sleep_for(100ms);

    std::cout << "sending other message by invalid type..." << std::endl;
    msgMessage.header.id = id++;
    msgMessage.header.type = static_cast<MessageType_t>(MessageType::queueUserType - 1);
    c1->send(static_cast<std::string>(packetMessage));
    std::this_thread::sleep_for(100ms);

    // TODO check results
  }
  SECTION("rejected requests") {
    // TODO implement this
    REQUIRE(true);
  }
}
