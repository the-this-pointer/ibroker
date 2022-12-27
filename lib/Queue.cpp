#include "Queue.h"
#include "ConnectionHandler.h"

using namespace thisptr::broker;

void Queue::publish(const std::shared_ptr<MessagePacket> &packet) {
  m_messages.push_back(packet);
  routePackets();
}

void Queue::addConnection(std::shared_ptr<ClientSocket> connection) {
  m_connections.push_back(connection);
}

void Queue::routePackets() {
  if (m_connections.empty())
    return;

  auto it = m_messages.begin();
  auto itc = m_connections.begin();
  for(; it != m_messages.end();) {
    std::string payload = static_cast<std::string>(**it);
    for(itc = m_connections.begin(); itc != m_connections.end();) {
      if(auto c = itc->lock()) {
        if (c->queue())
          (*c).send(payload);
        itc++;
      } else
        itc = m_connections.erase(itc);
    }
    it = m_messages.erase(it);
  }
}
