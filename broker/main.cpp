#include "../lib/ConnectionHandler.h"

using namespace thisptr::net;
using namespace thisptr::broker;

int main()
{
  TcpServer<BrokerConnectionHandler> server;
  server.start("127.0.0.1", "7232");

  return 0;
}
