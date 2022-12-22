#ifndef IBROKER_CONNECTIONHANDLER_H
#define IBROKER_CONNECTIONHANDLER_H

#include <Net.h>
#include "QueueManager.h"

namespace thisptr
{
  namespace broker
  {
    using namespace thisptr::net;
    class ServerHandler: public std::enable_shared_from_this<ServerHandler>,
                         public AsyncConnectionHandlerBase<AsioTcpSocket<ServerHandler>> {
    public:
      virtual ~ServerHandler() = default;
      void onDisconnected(asio::ip::tcp::socket& sock) override;
      void onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;
      void onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;
      void onNewConnection(asio::ip::tcp::socket& sock) override;

    private:
      std::shared_ptr<Queue> m_queue;
      std::string m_data;
      std::unordered_map<asio::ip::tcp::socket *, std::shared_ptr<AsioTcpSocket<ServerHandler>>> m_connections;
    };

    /*class ServerHandler: public std::enable_shared_from_this<ServerHandler>,
        public AsyncConnectionHandlerBase<AsioTcpSocket<ServerHandler>> {
    public:
      virtual ~ServerHandler();
      void onDisconnected(asio::ip::tcp::socket &sock) override;
      void onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;
      void onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;
      void onNewConnection(asio::ip::tcp::socket& sock) override;

    private:
      std::shared_ptr<Queue> m_queue;
      std::string m_data;
      std::unordered_map<asio::ip::tcp::socket *, std::shared_ptr<AsioTcpSocket<ServerHandler>>> m_connections;
    };*/

  }
}

#endif //IBROKER_CONNECTIONHANDLER_H
