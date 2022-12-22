
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


TEST_CASE("queue declare sample", "[handler]") {
  SECTION("successfull declaration") {
    TestQueueManager qm(QueueManager::instance());
    REQUIRE(qm.queues().size() == 0);
    REQUIRE(qm.queueBindings().size() == 0);

    auto server = startServer();
    std::this_thread::sleep_for(1000ms);

    auto chandler = std::make_shared<ClientHandler>();
    AsioTcpSocket<ClientHandler> c(chandler);
    c.connect("127.0.0.1", "7232");

    Message msg;
    msg.type = queueDeclare;
    std::string data = "camera,takepic";
    msg.size = data.length() + sizeof msg.type;
    memcpy(msg.payload, data.data(), data.length());

    MessagePacket packet(msg, true);
    c.send(static_cast<std::string>(packet));
    c.recv();
    std::this_thread::sleep_for(3000ms);

    REQUIRE(qm.queues().size() == 1);
    REQUIRE(qm.queueBindings().size() == 1);
  }
}
