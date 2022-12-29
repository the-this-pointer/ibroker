#include "../lib/ConnectionHandler.h"
#include "../lib/ClientSocket.h"
#include "../lib/Logger.h"
#include "spdlog/spdlog.h"
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <Net.h>

using namespace thisptr::utils;
using namespace thisptr::net;
using namespace thisptr::broker;
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

int main() {
  thisptr::broker::Logger::init();
  LE("Hi from this broker!");

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

  return 0;
}
