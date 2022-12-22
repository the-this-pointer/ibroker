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

class ClientHandler: public AsyncConnectionHandlerBase<AsioTcpSocket<ClientHandler>> {
public:
  void onConnected(asio::ip::tcp::socket& sock, const std::string &endpoint) override {
    std::cout << "[client] connected to " << endpoint.c_str() << std::endl;
  }

  void onDisconnected(asio::ip::tcp::socket& sock) override {
    std::cout << "[client] disconnected" << std::endl;
  }

  void onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override {
    if (ec) {
      std::cerr << "[client] unable to read from socket, ec: " << ec << std::endl;
    } else
      std::cout << "[client] data received: " << payload.c_str() << std::endl;
  }

  void onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override {
    if (ec) {
      std::cerr << "[client] unable to write to socket" << std::endl;
    } else
      std::cout << "[client] data sent, len: " << payload.length() << std::endl;
  }

};

auto handler = std::make_shared<ServerHandler>();
AsyncTcpServer<ServerHandler> s(handler);

void stopServer() {
  // stop server after 10 seconds
  std::this_thread::sleep_for(10000ms);
  s.stop();
}

void client(int idx) {
  std::this_thread::sleep_for(1000ms);
  auto chandler = std::make_shared<ClientHandler>();
  AsioTcpSocket<ClientHandler> c(chandler);
  if (!c.connect("127.0.0.1", "7232"))
  {
    std::cout << idx << " : unable to connect to host" << std::endl;
    return;
  }

  Message msg;
  msg.type = queueDeclare;
  std::string data = "camera,takepic";
  msg.size = data.length() + sizeof msg.type;
  memcpy(msg.payload, data.data(), data.length());

  MessagePacket packet(msg, true);
  c.send(static_cast<std::string>(packet));

  c.recv();
  std::this_thread::sleep_for(3000ms);
}

int main() {
  s.start("127.0.0.1", "7232");

  client(1);
  std::this_thread::sleep_for(1000ms);

  return 0;
}
