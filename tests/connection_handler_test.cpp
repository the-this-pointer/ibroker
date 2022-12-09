
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../lib/ConnectionHandler.h"

using namespace thisptr;
using namespace thisptr::broker;
using namespace thisptr::net;

void stopServer(TcpServer<BrokerConnectionHandler>* s) {
  // stop server after 10 seconds
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(3000ms);
  s->stop();
}

void client(int idx) {
  TcpClient c;
  REQUIRE(c.connect("127.0.0.1", "7232"));

  std::stringstream ss;
  ss << idx << " : " << "hello!\r\n";
  REQUIRE(c.send(ss.str().c_str()) > 0);

  char buffer[256] = {0};
  REQUIRE(c.recv(buffer, 256) == 0);

  REQUIRE(c.close());
}

TEST_CASE("handler receives message as desired", "[handler]") {
  SECTION("success") {
    TcpServer<BrokerConnectionHandler> s;
    s.setNewHandler([]() -> std::shared_ptr<BrokerConnectionHandler> {
      return std::make_shared<BrokerConnectionHandler>();
    });
    s.start("127.0.0.1", "7232");

    std::vector<std::thread> threads;
    threads.emplace_back(stopServer, &s);
    threads.emplace_back(client, 0);
    for(auto& t: threads)
      t.join();

    REQUIRE(true);
  }
}
