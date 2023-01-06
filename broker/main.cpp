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

std::unique_ptr<AsyncTcpServer<ServerHandler>> server {nullptr};

void startServer() {
  std::shared_ptr<ServerHandler> handler = std::make_shared<ServerHandler>();
  server = std::make_unique<AsyncTcpServer<ServerHandler>>(handler);

  server->start("127.0.0.1", "7232");
  server->waitForFinished();
}

void stopServer() {
  server->stop();
}

int main() {
  thisptr::broker::Logger::init();
  LI("Starting broker...");
  startServer();
  return 0;
}
