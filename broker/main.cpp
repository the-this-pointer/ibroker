#include "../lib/ConnectionHandler.h"
#include "../lib/QueueManager.h"

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

int main()
{
  TestQueueManager qm(QueueManager::instance());

  std::shared_ptr<ServerHandler> handler = std::make_shared<ServerHandler>();
  auto server = std::make_shared<AsyncTcpServer<ServerHandler>>(handler);
  server->start("127.0.0.1", "7232");

  std::this_thread::sleep_for(1000ms);
  auto chandler = std::make_shared<ClientHandler>();
  AsioTcpSocket<ClientHandler> c(chandler);

  c.connect("127.0.0.1", "7232");
  std::this_thread::sleep_for(500ms);

  Message msg;
  msg.type = queueDeclare;
  std::string data = "camera,takepic";
  msg.size = data.length() + sizeof msg.type;
  memcpy(msg.payload, data.data(), data.length());

  MessagePacket packet(msg, true);
  c.send(static_cast<std::string>(packet));
  std::this_thread::sleep_for(500ms);

  c.recv();
  std::this_thread::sleep_for(500ms);

  c.close();

  server->stop();

  return 0;
}
