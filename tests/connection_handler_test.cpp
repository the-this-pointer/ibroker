
#define CATCH_CONFIG_MAIN
#include <memory>
#include "catch.hpp"
#include "../lib/ConnectionHandler.h"
#include "../lib/QueueManager.h"
#include "../lib/Message.h"

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

class ClientHandler: public AsyncConnectionHandlerBase<AsioTcpSocket<ClientHandler>> {
public:
  void onConnected(asio::ip::tcp::socket& sock, const std::string &endpoint) override {
    std::cout << "[client] connected to " << endpoint.c_str() << std::endl;
  }

  void onDisconnected(asio::ip::tcp::socket& sock) override {
    std::cout << "[client] disconnected" << std::endl;
  }

  bool onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override {
    if (ec) {
      std::cerr << "[client] unable to read from socket, ec: " << ec << std::endl;
      return false;
    } else
      std::cout << "[client] data received: " << payload << std::endl;
    return true;
  }

  void onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override {
    if (ec) {
      std::cerr << "[client] unable to write to socket" << std::endl;
    } else
      std::cout << "[client] data sent, len: " << payload.length() << std::endl;
  }
};

TEST_CASE("message packet test", "[packet]") {
  SECTION("sucess serialization") {
    Message_t msg;
    msg.header.id = 0x01;
    msg.header.type = MessageType::queueMessage;
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
    msg.header.type = MessageType::queueMessage;
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

    auto chandler = std::make_shared<ClientHandler>();
    AsyncTcpClient<ClientHandler> c(chandler);
    c.connect("127.0.0.1", "7232");

    uint8_t id;
    Message msg;
    msg.header.id = id++;
    msg.header.type = queueDeclare;
    std::string data = "camera,takepic";
    msg.setBody(data);

    MessagePacket packet(msg, true);
    c.send(static_cast<std::string>(packet));
    c.recv();
    std::this_thread::sleep_for(100ms);

    REQUIRE(qm.queues().size() == 1);
    REQUIRE(qm.queueBindings().size() == 1);
  }
  SECTION("successfull message match") {
    TestQueueManager qm(QueueManager::instance());

    auto server = startServer();
    std::this_thread::sleep_for(1000ms);

    auto chandler = std::make_shared<ClientHandler>();
    AsyncTcpClient<ClientHandler> c1(chandler), c2(chandler);
    c1.connect("127.0.0.1", "7232");
    c2.connect("127.0.0.1", "7232");
    c1.recv();
    c2.recv();

    // Declare Queue
    uint8_t id;
    Message msgDeclare;
    msgDeclare.header.id = id++;
    msgDeclare.header.type = queueDeclare;
    std::string data = "camera,takepic";
    msgDeclare.setBody(data);

    MessagePacket packetDeclare(msgDeclare, true);
    c1.send(static_cast<std::string>(packetDeclare));
    std::this_thread::sleep_for(100ms);

    // Bind Queue
    Message msgBind;
    msgBind.header.id = id++;
    msgBind.header.type = queueBind;
    std::string queueName = "camera";
    msgBind.setBody(queueName);

    MessagePacket packetBind(msgBind, true);
    c1.send(static_cast<std::string>(packetBind));
    c2.send(static_cast<std::string>(packetBind));
    std::this_thread::sleep_for(100ms);

    // Post Message To Queue
    Message msgMessage;
    msgMessage.header.id = id++;
    msgMessage.header.type = queueMessage;
    std::string messagePayload = "takepic,hi this is sample payload for take picture!";
    msgMessage.setBody(messagePayload);

    MessagePacket packetMessage(msgMessage, true);
    c1.send(static_cast<std::string>(packetMessage));
    std::this_thread::sleep_for(100ms);

    // TODO check results
  }
  SECTION("rejected requests") {
    // TODO implement this
    REQUIRE(true);
  }
}
