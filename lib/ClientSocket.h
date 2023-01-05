#ifndef IBROKER_CLIENTSOCKET_H
#define IBROKER_CLIENTSOCKET_H

#include <memory>
#include "../lib/ConnectionHandler.h"
#include "../lib/QueueManager.h"
#include "../lib/MessagePacket.h"

namespace thisptr {
  namespace broker {

    class ClientSocket: public std::enable_shared_from_this<ClientSocket>,
                        public ::thisptr::net::AsyncConnectionHandlerBase<::thisptr::net::AsioTcpSocket<ClientSocket>>,
                        public ::thisptr::net::AsyncTcpClient<ClientSocket> {
    public:
      typedef enum: uint8_t {
        WaitMessage,
        ReadHeader,
        ReadBody
      } HandlerStatus_t;

      ClientSocket();
      void initialize();
      void onConnected(asio::ip::tcp::socket& sock, const std::string &endpoint) override;
      void onDisconnected(asio::ip::tcp::socket& sock) override;
      bool onDataReceived(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& data) override;
      void onDataSent(asio::ip::tcp::socket& sock, std::error_code ec, const std::string& payload) override;
    private:
      inline void waitForMessage() { m_status = WaitMessage; recv(MessageIndicatorLength); }

      inline void readHeader() {
        m_status = ReadHeader;
        recv_until(BodyIndicator);
      }

      inline void readBody(const MessageSize_t& size) { m_status = ReadBody; recv(size); }

    private:
      std::string m_data;
      HandlerStatus_t m_status {WaitMessage};
    };
  }
}


#endif //IBROKER_CLIENTSOCKET_H
