#ifndef IBROKER_CONNECTIONHANDLER_H
#define IBROKER_CONNECTIONHANDLER_H

#include <Net.h>

#include <utility>
#include "QueueManager.h"

namespace thisptr
{
  namespace broker
  {
    using namespace thisptr::net;

    class ServerHandler;
    class ClientSocket: public AsioTcpSocket<ServerHandler> {
    public:
      ClientSocket(std::shared_ptr<ServerHandler> handler, asio::ip::tcp::socket& socket):
          AsioTcpSocket<ServerHandler>(handler, socket)
      {}

      void setQueue(std::shared_ptr<Queue> queue) { m_queue = std::move(queue); }
      std::shared_ptr<Queue> queue() { return m_queue; }
    private:
      std::shared_ptr<Queue> m_queue;
    };

    class ServerHandler: public std::enable_shared_from_this<ServerHandler>,
                         public AsyncConnectionHandlerBase<AsioTcpSocket<ServerHandler>> {
    public:
      virtual ~ServerHandler() = default;
      void onDisconnected(asio::ip::tcp::socket& sock) override;
      bool onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;
      void onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;
      void onNewConnection(asio::ip::tcp::socket& sock) override;

    private:
      std::string m_data;
      std::unordered_map<asio::ip::tcp::socket *, std::shared_ptr<ClientSocket>> m_connections;
    };
  }
}

#endif //IBROKER_CONNECTIONHANDLER_H
