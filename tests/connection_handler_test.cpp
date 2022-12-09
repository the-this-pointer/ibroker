
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../lib/ConnectionHandler.h"
#include "../lib/Message.h"

using namespace thisptr;
using namespace thisptr::broker;
using namespace thisptr::net;

void stopServer(TcpServer<BrokerConnectionHandler>* s) {
  // stop server after 10 seconds
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(3000ms);
  s->stop();
}

void client(int idx, bool* res) {
  TcpClient c;
  REQUIRE(c.connect("127.0.0.1", "7232"));

  Message msg;
  msg.type = queueDeclare;
  msg.size = 10;
  std::string data = "hi there!";
  memcpy(msg.payload, data.data(), data.length());

  MessagePacket packet(msg, true);
  std::string strPacket = static_cast<std::string>(packet);

  REQUIRE(c.send(strPacket.c_str(), strPacket.length()) > 0);

  char buffer[256] = {0};
  if (c.recv(buffer, 256) > 0)
    *res = true;

  REQUIRE_FALSE(c.close());
}

TEST_CASE("handler receives message as desired", "[handler]") {
  SECTION("successfull parsing message") {
    TcpServer<BrokerConnectionHandler> s;
    s.setNewHandler([]() -> std::shared_ptr<BrokerConnectionHandler> {
      return std::make_shared<BrokerConnectionHandler>();
    });
    s.start("127.0.0.1", "7232");

    std::vector<std::thread> threads;
    bool clientRes = false;
    threads.emplace_back(stopServer, &s);
    threads.emplace_back(client, 0, &clientRes);
    for(auto& t: threads)
      t.join();

    REQUIRE_FALSE(clientRes);
    REQUIRE(true);
  }
}
