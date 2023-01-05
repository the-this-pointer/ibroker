#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "spdlog/spdlog.h"
#include <memory>
#include <string>
#include "../lib/ClientSocket.h"
#include "../lib/ConnectionHandler.h"
#include "../lib/QueueManager.h"
#include "../lib/MessagePacket.h"
#include "../lib/Logger.h"

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

TEST_CASE("success serialization") {
  thisptr::broker::Logger::init();

  Message_t msg;
  msg.header.id = 0x01;
  msg.header.type = static_cast<MessageType_t>(MessageType::queueUserType + 1);
  msg.header.topic = "sample topic";
  msg.setBody("this is a test!");

  MessagePacket packet(msg, true);
  std::string dataToSend = static_cast<std::string>(packet);

  Message_t msg2;
  MessagePacket packet2(msg2, true);
  packet2.fromString(dataToSend);

  CHECK(*&msg.header.id == *&msg2.header.id);
  CHECK(*&msg.header.type == *&msg2.header.type);
  CHECK(*&msg.header.size == *&msg2.header.size);
  CHECK(*&msg.header.topic == *&msg2.header.topic);
  CHECK(memcmp(msg.body.data(), msg2.body.data(), msg.header.size) == 0);
}

TEST_CASE("failed serialization") {
  thisptr::broker::Logger::init();

  Message_t msg;
  msg.header.id = 0x01;
  msg.header.type = static_cast<MessageType_t>(MessageType::queueUserType + 1);
  msg.setBody("this is a test!");

  MessagePacket packet(msg, true);
  std::string dataToSend = static_cast<std::string>(packet);

  Message_t msg2;
  MessagePacket packet2(msg2);
  packet2.fromString(dataToSend);

  CHECK(*&msg.header.id != *&msg2.header.id);
}

TEST_CASE("successfull declaration") {
  thisptr::broker::Logger::init();

  TestQueueManager qm(QueueManager::instance());
  CHECK(qm.queues().size() == 0);
  CHECK(qm.queueBindings().size() == 0);

  auto server = startServer();
  std::this_thread::sleep_for(100ms);

  auto c = std::make_shared<ClientSocket>();
  c->initialize();
  c->connect("127.0.0.1", "7232");

  uint8_t id;
  Message msg;
  msg.header.id = id++;
  msg.header.type = queueDeclare;
  msg.header.topic = "takepic";
  std::string data = "camera";
  msg.setBody(data);

  MessagePacket packet(msg, true);
  c->send(static_cast<std::string>(packet));
  std::this_thread::sleep_for(1000ms);

  CHECK(qm.queues().size() == 1);
  CHECK(qm.queueBindings().size() == 1);
}

TEST_CASE("successfull message match") {
  thisptr::broker::Logger::init();

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
  msgDeclare.header.topic = "takepic";
  std::string data = "camera";
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
  msgMessage.header.topic = "takepic";
  std::string messagePayload = "hi this is sample payload for take picture!";
  msgMessage.setBody(messagePayload);

  MessagePacket packetMessage(msgMessage, true);
  c1->send(static_cast<std::string>(packetMessage));
  std::this_thread::sleep_for(100ms);

  LT("closing c2");
  c2->close();
  std::this_thread::sleep_for(1000ms);

  LT("sending other message by c1...");
  c1->send(static_cast<std::string>(packetMessage));
  std::this_thread::sleep_for(100ms);

  LT("sending other message by invalid type...");
  msgMessage.header.id = id++;
  msgMessage.header.type = static_cast<MessageType_t>(MessageType::queueUserType - 1);
  c1->send(static_cast<std::string>(packetMessage));
  std::this_thread::sleep_for(100ms);

  // TODO check results
}

TEST_CASE("rejected requests") {
  thisptr::broker::Logger::init();

  // TODO implement this
  CHECK(true);
}
