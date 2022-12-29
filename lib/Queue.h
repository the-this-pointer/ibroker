#ifndef IBROKER_QUEUE_H
#define IBROKER_QUEUE_H

#include "MessagePacket.h"
#include "MessageQueue.h"
#include <mutex>
#include <memory>

namespace thisptr
{
  namespace broker {
    class ClientConnection;
    class Queue {
    public:
      virtual void publish(const std::shared_ptr<MessagePacket> &packet);
      virtual void addConnection(std::shared_ptr<ClientConnection> connection);
      virtual void routePackets();

    protected:
      MessageQueue <std::weak_ptr<ClientConnection>> m_connections;
      MessageQueue <std::shared_ptr<MessagePacket>> m_messages;
    };
  }
}

#endif //IBROKER_QUEUE_H
