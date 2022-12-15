#include "../lib/ConnectionHandler.h"
#include "../lib/QueueManager.h"

using namespace thisptr::net;
using namespace thisptr::broker;

int main()
{
  TcpServer<BrokerConnectionHandler> server;
  server.setNewHandler([]() -> std::shared_ptr<BrokerConnectionHandler> {
    return std::make_shared<BrokerConnectionHandler>();
  });
  server.start("127.0.0.1", "7232");

  return 0;
}
