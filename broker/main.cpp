#include "../lib/ConnectionHandler.h"
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

int main() {
  TestQueueManager qm(QueueManager::instance());

  auto server = startServer();
  std::this_thread::sleep_for(1000ms);

  auto chandler = std::make_shared<ClientHandler>();
  AsioTcpSocket<ClientHandler> c1(chandler), c2(chandler);
  c1.connect("127.0.0.1", "7232");
  c2.connect("127.0.0.1", "7232");
  c1.recv();
  c2.recv();

  // Declare Queue
  Message msgDeclare;
  msgDeclare.type = queueDeclare;
  std::string data = "camera,takepic";
  msgDeclare.size = data.length() + sizeof msgDeclare.type;
  memcpy(msgDeclare.payload, data.data(), data.length());

  MessagePacket packetDeclare(msgDeclare, true);
  c1.send(static_cast<std::string>(packetDeclare));
  std::this_thread::sleep_for(1000ms);

  // Bind Queue
  Message msgBind;
  msgBind.type = queueBind;
  std::string queueName = "camera";
  msgBind.size = queueName.length() + sizeof msgBind.type;
  memcpy(msgBind.payload, queueName.data(), queueName.length());

  MessagePacket packetBind(msgBind, true);
  c1.send(static_cast<std::string>(packetBind));
  c2.send(static_cast<std::string>(packetBind));

  std::this_thread::sleep_for(1000ms);

  // Post Message To Queue

  Message msgMessage;
  msgMessage.type = message;
  std::string messagePayload = "takepic,hi this is sample payload for take picture!";
  msgMessage.size = messagePayload.length() + sizeof msgMessage.type;
  memcpy(msgMessage.payload, messagePayload.data(), messagePayload.length());

  MessagePacket packetMessage(msgMessage, true);
  c1.send(static_cast<std::string>(packetMessage));
  std::this_thread::sleep_for(1000ms);

  return 0;
}
