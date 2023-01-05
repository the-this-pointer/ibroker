#ifndef IBROKER_CONNECTIONHANDLER_H
#define IBROKER_CONNECTIONHANDLER_H

#include <Net.h>

#include <utility>
#include "QueueManager.h"

namespace thisptr
{
  namespace broker
  {
    class ClientConnection: public std::enable_shared_from_this<ClientConnection>,
                        public ::thisptr::net::AsyncConnectionHandlerBase<::thisptr::net::AsioTcpSocket<ClientConnection>>,
                        public ::thisptr::net::AsioTcpSocket<ClientConnection> {
    public:
      typedef enum: uint8_t {
        WaitMessage,
        ReadHeader,
        ReadBody
      } HandlerStatus_t;

      explicit ClientConnection(asio::ip::tcp::socket& socket):
          AsioTcpSocket<ClientConnection>(socket), m_status(WaitMessage)
      {}

      void initialize();

      virtual ~ClientConnection() = default;
      void onDisconnected(asio::ip::tcp::socket& sock) override;
      bool onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& data) override;
      void onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;

      void setQueue(std::shared_ptr<Queue> queue) {
        m_queue = std::move(queue);
        m_queue->addConnection(this->shared_from_this());
      }

      std::shared_ptr<Queue> queue() { return m_queue; }
    private:
      inline void waitForMessage() { m_status = WaitMessage; recv(MessageIndicatorLength); }

      inline void readHeader() {
        m_status = ReadHeader;
        recv_until(BodyIndicator);
      }

      inline void readBody(const MessageSize_t& size) { m_status = ReadBody; recv(size); }

    private:
      std::string m_data;
      std::shared_ptr<Queue> m_queue;
      HandlerStatus_t m_status;
    };

    class ServerHandler: public std::enable_shared_from_this<ServerHandler>,
                         public ::thisptr::net::AsyncConnectionHandlerBase<::thisptr::net::AsioTcpSocket<ServerHandler>> {
    public:
      virtual ~ServerHandler() = default;
      void onDisconnected(asio::ip::tcp::socket& sock) override;
      bool onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;
      void onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;
      void onNewConnection(asio::ip::tcp::socket& sock) override;

    private:
      std::unordered_map<asio::ip::tcp::socket *, std::shared_ptr<ClientConnection>> m_connections;
    };
  }
}

#endif //IBROKER_CONNECTIONHANDLER_H
