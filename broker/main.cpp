#include <Net.h>

using namespace thisptr;

int main()
{
  net::TcpServer server;

  server.setConnectCallback([](const std::shared_ptr<thisptr::net::TcpSocket>& conn){
    std::cout << "new connection!!" << std::endl;
  });
  server.setDisconnectCallback([](const std::shared_ptr<thisptr::net::TcpSocket>& conn){
    std::cout << "client disconnected!!" << std::endl;
  });
  server.setMessageCallback(onMessageCallback);
  server.start("127.0.0.1", "7232");



  return 0;
}
