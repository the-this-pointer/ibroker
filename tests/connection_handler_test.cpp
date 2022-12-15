
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../lib/ConnectionHandler.h"
#include "../lib/QueueManager.h"
#include "../lib/Message.h"

using namespace thisptr;
using namespace thisptr::broker;
using namespace thisptr::net;

std::shared_ptr<TcpServer<BrokerConnectionHandler>> startServer() {
  std::shared_ptr<TcpServer<BrokerConnectionHandler>> s = std::make_shared<TcpServer<BrokerConnectionHandler>>();
  s->setNewHandler([]() -> std::shared_ptr<BrokerConnectionHandler> {
    return std::make_shared<BrokerConnectionHandler>();
  });
  s->start("127.0.0.1", "7232");
  return s;
}

void stopServer(const std::shared_ptr<TcpServer<BrokerConnectionHandler>>& s) {
  // stop server after 10 seconds
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(3000ms);
  s->stop();
}

TEST_CASE("queue declare sample", "[handler]") {
  SECTION("successfull declaration") {
    TestQueueManager qm(QueueManager::instance());
    REQUIRE(qm.queues().size() == 0);
    REQUIRE(qm.queueBindings().size() == 0);

    auto s = startServer();

    std::vector<std::thread> threads;
    threads.emplace_back(stopServer, s);

    TcpClient c;
    REQUIRE(c.connect("127.0.0.1", "7232"));
    Message msg;
    msg.type = queueDeclare;
    std::string data = "camera,takepic";
    msg.size = data.length() + sizeof msg.type;
    memcpy(msg.payload, data.data(), data.length());

    MessagePacket packet(msg, true);
    std::string strPacket = static_cast<std::string>(packet);

    REQUIRE(c.send(strPacket.c_str(), strPacket.length()) > 0);

    char buffer[256] = {0};
    c.recv(buffer, 256);
    REQUIRE_FALSE(c.close());

    for(auto& t: threads)
      t.join();

    REQUIRE(qm.queues().size() == 1);
    REQUIRE(qm.queueBindings().size() == 1);
  }
}
