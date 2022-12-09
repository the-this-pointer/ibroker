#ifndef IBROKER_CONNECTIONHANDLER_H
#define IBROKER_CONNECTIONHANDLER_H

#include <Net.h>

namespace thisptr
{
  namespace broker
  {
    class BrokerConnectionHandler: public net::ConnectionHandlerBase {
    public:
      void onConnect() override;
      void onDisconnect() override;
      void onMessage(std::string data) override;

    private:
      std::string m_data;
    };
  }
}

#endif //IBROKER_CONNECTIONHANDLER_H
